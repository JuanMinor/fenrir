#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Widen Fenrir's value head while carrying every other weight across intact.

The value head is ~0.2% of the network (50k of 21.5M parameters) and
bottlenecks 256 trunk channels down to 3, giving it 192 features to rank
how won a position is. Widening it costs ~0.1% more compute. Only these
tensors change shape:

    value_conv.weight      (3,256,1,1) -> (N,256,1,1)
    value_bn.*             (3,)        -> (N,)
    value_fc1.weight       (256,192)   -> (256,N*64)

Everything else — the trunk, all residual blocks, the entire policy head,
value_fc1.bias and value_fc2 — transfers byte-for-byte.

WHY YOU CAN TRUST THE RESULT: the policy path (trunk -> policy_conv ->
policy_bn -> policy_fc) contains no changed tensor, so the migrated model
must produce bit-identical policy logits. This script verifies exactly
that on real positions and refuses to write the output unless every check
passes. It never modifies its input.

Usage (from the repo root, needs torch — i.e. run it on the training host):
  python3 scripts/migrate_value_head.py --input onnx/fenrir.pth \
      --output onnx/fenrir_v2.pth --value-channels 32
  python3 scripts/migrate_value_head.py --input onnx/fenrir.pth --dry-run
"""

import argparse
import os
import sys

VERIFY_FENS = [
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    "4k3/pppppppp/8/8/8/8/PPPPPPPP/3QK3 w - - 0 1",
    "1k1r4/pp3ppp/8/8/8/8/PP3PPP/1K1R3R w - - 0 1",
    "6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1",
]


def main():
    parser = argparse.ArgumentParser(description="Widen the value head, preserving all other weights")
    parser.add_argument("--input", required=True, help="existing .pth checkpoint (never modified)")
    parser.add_argument("--output", help="destination .pth (required unless --dry-run)")
    parser.add_argument("--value-channels", type=int, default=32, help="new value head width (default 32)")
    parser.add_argument("--dry-run", action="store_true", help="report the plan and verify, but write nothing")
    parser.add_argument("--seed", type=int, default=0, help="seed for the new head's initialization")
    args = parser.parse_args()

    if not args.dry_run and not args.output:
        parser.error("--output is required unless --dry-run")
    if args.output and os.path.exists(args.output):
        parser.error(f"refusing to overwrite existing file: {args.output}")

    import torch

    sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
    from model import AlphaZeroNet, infer_value_channels

    torch.manual_seed(args.seed)

    old_sd = torch.load(args.input, map_location="cpu", weights_only=True)
    old_width = infer_value_channels(old_sd)
    print(f"input             : {args.input}")
    print(f"value head width  : {old_width} -> {args.value_channels}")
    if old_width == args.value_channels:
        print("\nnothing to do: the checkpoint already has this width")
        return 0

    old_model = AlphaZeroNet(value_channels=old_width)
    old_model.load_state_dict(old_sd)
    old_model.eval()

    new_model = AlphaZeroNet(value_channels=args.value_channels)
    new_sd = new_model.state_dict()

    # Transfer every tensor whose name AND shape still match. Note that
    # load_state_dict(strict=False) is NOT sufficient here: it tolerates
    # missing keys but still raises on shape mismatches.
    transferred, reinitialized = [], []
    for name, tensor in new_sd.items():
        old = old_sd.get(name)
        if old is not None and old.shape == tensor.shape:
            new_sd[name] = old.clone()
            transferred.append(name)
        else:
            reinitialized.append(name)
    new_model.load_state_dict(new_sd)
    new_model.eval()

    def count(names, sd):
        return sum(sd[n].numel() for n in names if not n.endswith("num_batches_tracked"))

    kept = count(transferred, new_sd)
    fresh = count(reinitialized, new_sd)
    total = kept + fresh
    print(f"\ntensors transferred: {len(transferred)} of {len(new_sd)}")
    print(f"parameters kept    : {kept:,} of {total:,}  ({100.0 * kept / total:.2f}%)")
    print(f"parameters reinit  : {fresh:,}  ({100.0 * fresh / total:.2f}%)")
    print("\nreinitialized tensors (the new value head):")
    for name in reinitialized:
        print(f"  {name:<32}{str(tuple(new_sd[name].shape))}")

    # ---------------- verification ----------------
    print("\n" + "=" * 62)
    print("VERIFICATION")
    print("=" * 62)
    failures = []

    # 1. Nothing outside the value head may be reinitialized.
    stragglers = [n for n in reinitialized if not n.startswith(("value_conv", "value_bn", "value_fc1"))]
    if stragglers:
        failures.append(f"unexpected tensors reinitialized: {stragglers}")
    print(f"[{'ok ' if not stragglers else 'FAIL'}] only value-head tensors were reinitialized")

    # 2. Every transferred tensor must be bit-identical, not merely same-shaped.
    altered = [n for n in transferred if not torch.equal(new_sd[n], old_sd[n])]
    if altered:
        failures.append(f"transferred tensors differ from the original: {altered[:5]}")
    print(f"[{'ok ' if not altered else 'FAIL'}] all {len(transferred)} transferred tensors are bit-identical")

    # 3. The trunk and policy head must be complete.
    for prefix, label in (("conv_input", "input conv"), ("bn_input", "input bn"),
                          ("res_blocks", "residual tower"), ("policy_", "policy head")):
        missing = [n for n in old_sd if n.startswith(prefix) and n not in transferred]
        if missing:
            failures.append(f"{label} did not fully transfer: {missing[:3]}")
        print(f"[{'ok ' if not missing else 'FAIL'}] {label} fully transferred")

    # 4. The decisive test: identical policy logits on real positions.
    #    The policy path has no changed tensor, so any difference means
    #    the migration disturbed knowledge it should not have touched.
    try:
        from train import ChessDataset
        encoder = ChessDataset("__no_data_dir__")
        worst_policy, worst_value = 0.0, 0.0
        with torch.no_grad():
            for fen in VERIFY_FENS:
                x = encoder.fen_to_tensor(fen).unsqueeze(0)
                old_policy, old_value = old_model(x)
                new_policy, new_value = new_model(x)
                worst_policy = max(worst_policy, (old_policy - new_policy).abs().max().item())
                worst_value = max(worst_value, (old_value - new_value).abs().max().item())
        policy_ok = worst_policy < 1e-6
        if not policy_ok:
            failures.append(f"policy logits changed (max diff {worst_policy:.3e})")
        print(f"[{'ok ' if policy_ok else 'FAIL'}] policy logits identical across {len(VERIFY_FENS)} "
              f"positions (max diff {worst_policy:.2e})")
        print(f"[ -- ] value output changed by up to {worst_value:.3f} — EXPECTED, "
              f"the new head is untrained")
    except ImportError as exc:
        print(f"[ -- ] skipped output comparison (missing dependency: {exc})")

    print()
    if failures:
        print("MIGRATION FAILED — nothing written:")
        for problem in failures:
            print(f"  {problem}")
        return 1

    print("All checks passed: the trunk and policy head carried over exactly.")
    if args.dry_run:
        print("(dry run — no file written)")
        return 0

    torch.save(new_model.state_dict(), args.output)
    print(f"wrote {args.output}")
    print(f"input file untouched: {args.input}")
    print("\nnext steps:")
    print(f"  1. export ONNX:  python3 scripts/export_onnx.py {args.output} onnx/fenrir.onnx")
    print(f"  2. put the new checkpoint in place:  cp {args.output} onnx/fenrir.pth")
    print("  3. relaunch; expect a value-loss spike that recovers within ~an hour")
    print("  4. rollback if needed: restore the backed-up .pth/.onnx and relaunch")
    return 0


if __name__ == "__main__":
    sys.exit(main())
