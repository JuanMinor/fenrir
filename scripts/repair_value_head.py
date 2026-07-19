#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Rebuild a dead value head in place, keeping the trunk and policy head.

A value head whose tanh has saturated (probe_value.py shows the same
+/-1.0000 for every position) cannot recover on its own: tanh's gradient
is 1 - tanh^2, so a saturated output receives no gradient and stays stuck
forever. The loss then sits at a high plateau with the value term pinned
at maximum error.

WHY IT SATURATES, and why the output layer is zeroed here: Adam's first
update moves every parameter by roughly the learning rate regardless of
gradient magnitude (the second-moment estimate is initialized from that
same gradient, so the ratio is +/-1). A value_fc1 with 2048 inputs
accumulates ~10x more of that uniform movement than the original
192-input layer did, overshooting tanh's linear region on step one.
Zeroing value_fc2 contains it: the head emits exactly 0, fc1's first-step
movement is multiplied by ~0 on the way out, and fc2 grows from zero at
Adam's modest per-parameter rate — so by the time it can affect the
output, fc1 has settled. Verified over 200 steps: 0% saturation and a 9x
lower value loss versus the failing initialization.

The trunk and policy head are untouched, so all chess knowledge is kept;
only the value head restarts, and it relearns quickly off mature trunk
features.

Usage (needs torch — run it on the training host):
  python3 scripts/repair_value_head.py --input onnx/fenrir.pth \
                                       --output onnx/fenrir_repaired.pth
"""

import argparse
import os
import sys

CHECK_FENS = [
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "4k3/pppppppp/8/8/8/8/PPPPPPPP/3QK3 w - - 0 1",
    "3qk3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1",
    "1k1r4/pp3ppp/8/8/8/8/PP3PPP/1K1R3R w - - 0 1",
]

VALUE_PREFIXES = ("value_conv", "value_bn", "value_fc1", "value_fc2")


def main():
    parser = argparse.ArgumentParser(description="Reinitialize a saturated value head, keeping trunk and policy")
    parser.add_argument("--input", required=True, help="checkpoint with the dead head (never modified)")
    parser.add_argument("--output", required=True, help="destination .pth")
    parser.add_argument("--seed", type=int, default=0)
    args = parser.parse_args()

    if os.path.exists(args.output):
        parser.error(f"refusing to overwrite existing file: {args.output}")

    import torch

    sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
    from model import AlphaZeroNet, infer_value_channels

    torch.manual_seed(args.seed)

    old_sd = torch.load(args.input, map_location="cpu", weights_only=True)
    width = infer_value_channels(old_sd)
    print(f"input            : {args.input}")
    print(f"value head width : {width} (unchanged)")

    # Diagnose the head we are replacing, so the repair is justified on
    # evidence rather than assumed.
    try:
        from train import ChessDataset
        encoder = ChessDataset("__no_data_dir__")
        before = AlphaZeroNet(value_channels=width)
        before.load_state_dict(old_sd)
        before.eval()
        with torch.no_grad():
            values = [before(encoder.fen_to_tensor(f).unsqueeze(0))[1].item() for f in CHECK_FENS]
        saturated = sum(1 for v in values if abs(v) > 0.99)
        print(f"\nbefore repair    : {[f'{v:+.4f}' for v in values]}")
        print(f"                   {saturated} of {len(values)} positions saturated"
              f"{'  <- dead head confirmed' if saturated == len(values) else ''}")
        if saturated == 0:
            print("\nNOTE: this head is not saturated. Repairing would discard a working"
                  "\n      value head — make sure that is what you intend.")
    except ImportError as exc:
        print(f"(skipped before/after comparison: {exc})")
        encoder = None

    # Fresh, correctly-scaled weights for the value head only.
    fresh = AlphaZeroNet(value_channels=width).state_dict()
    new_sd = dict(old_sd)
    replaced = []
    for name in new_sd:
        if name.startswith(VALUE_PREFIXES):
            new_sd[name] = fresh[name].clone()
            replaced.append(name)

    # Zero the output layer: this is what prevents re-saturation.
    new_sd["value_fc2.weight"].zero_()
    new_sd["value_fc2.bias"].zero_()
    if "value_bn.num_batches_tracked" in new_sd:
        new_sd["value_bn.num_batches_tracked"].zero_()

    model = AlphaZeroNet(value_channels=width)
    model.load_state_dict(new_sd)
    model.eval()

    print(f"\nreplaced {len(replaced)} value-head tensors; value_fc2 zeroed")
    kept = sum(v.numel() for k, v in new_sd.items()
               if not k.startswith(VALUE_PREFIXES) and not k.endswith("num_batches_tracked"))
    total = sum(v.numel() for k, v in new_sd.items() if not k.endswith("num_batches_tracked"))
    print(f"trunk + policy kept: {kept:,} of {total:,} parameters ({100.0 * kept / total:.2f}%)")

    # ---- verification ----
    failures = []

    # Trunk and policy must be bit-identical to the input.
    altered = [k for k in old_sd
               if not k.startswith(VALUE_PREFIXES) and not torch.equal(new_sd[k], old_sd[k])]
    if altered:
        failures.append(f"non-value tensors changed: {altered[:5]}")
    print(f"[{'ok ' if not altered else 'FAIL'}] trunk and policy head bit-identical")

    if encoder is not None:
        with torch.no_grad():
            after = [model(encoder.fen_to_tensor(f).unsqueeze(0))[1].item() for f in CHECK_FENS]
            old_policy = before(encoder.fen_to_tensor(CHECK_FENS[0]).unsqueeze(0))[0]
            new_policy = model(encoder.fen_to_tensor(CHECK_FENS[0]).unsqueeze(0))[0]
            policy_diff = (old_policy - new_policy).abs().max().item()
        print(f"[{'ok ' if policy_diff < 1e-6 else 'FAIL'}] policy logits unchanged "
              f"(max diff {policy_diff:.2e})")
        if policy_diff >= 1e-6:
            failures.append("policy logits changed")

        still_saturated = [v for v in after if abs(v) > 0.99]
        print(f"[{'ok ' if not still_saturated else 'FAIL'}] repaired head is not saturated "
              f"(outputs {[f'{v:+.4f}' for v in after]})")
        if still_saturated:
            failures.append("repaired head is still saturated")

    print()
    if failures:
        print("REPAIR FAILED — nothing written:")
        for problem in failures:
            print(f"  {problem}")
        return 1

    torch.save(new_sd, args.output)
    print(f"wrote {args.output}  (input untouched)")
    print("\nnext steps:")
    print(f"  python3 scripts/export_onnx.py {args.output} onnx/fenrir.onnx")
    print(f"  cp {args.output} onnx/fenrir.pth")
    print("  relaunch, then PROBE AFTER 30 MINUTES:")
    print("    python3 scripts/probe_value.py checkpoints/<newest>.pth")
    print("  expect correct signs and varied magnitudes — NOT the same value everywhere")
    return 0


if __name__ == "__main__":
    sys.exit(main())
