#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Deep inspection of Fenrir models: .pth checkpoints, .onnx exports, and the
trainer's loss history.

A .pth state_dict carries more than the ONNX export does — full-precision
weights, BatchNorm running statistics (a window into the activation
distributions seen during training), and num_batches_tracked (exactly how
many training batches the net has ever consumed). Diffing two checkpoints
shows which layers are still moving and which have converged, which is the
closest thing to a per-layer learning curve that weights alone can provide.

Sections produced (each independent; failures are isolated and reported):
  architecture   inferred topology, parameter counts, memory, FLOPs/eval
  parameters     per-component weight statistics, sparsity, outlier flags
  batchnorm      gamma/beta health, running stats, dead-channel detection
  heads          policy/value head detail, singular-value spectrum, rank
  behavior       value + policy outputs on canned positions (needs torch
                 and python-chess)
  compare        per-layer drift between checkpoints (pass 2+ .pth files)
  onnx           graph structure, opset, op histogram, IO shapes
  losses         parsed learning curve from the trainer's log

Usage:
  python3 scripts/inspect_model.py onnx/fenrir.pth
  python3 scripts/inspect_model.py checkpoints/*.pth --train-log logs/train.log
  python3 scripts/inspect_model.py onnx/fenrir.pth --onnx onnx/fenrir.onnx --all-tensors
"""

import argparse
import math
import os
import re
import sys
import time
from collections import OrderedDict, defaultdict

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

# Canned positions for the behavioral probe: material situations with known
# correct verdicts, so a trained value head is easy to distinguish from an
# untrained one.
PROBE_POSITIONS = [
    ("start position (expect value ~0)", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
    ("white +Q, white to move (expect >> 0)", "4k3/pppppppp/8/8/8/8/PPPPPPPP/3QK3 w - - 0 1"),
    ("white +Q, black to move (expect << 0)", "4k3/pppppppp/8/8/8/8/PPPPPPPP/3QK3 b - - 0 1"),
    ("black +Q, white to move (expect << 0)", "3qk3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1"),
    ("white +1R midgame, white to move (expect > 0)", "1k1r4/pp3ppp/8/8/8/8/PP3PPP/1K1R3R w - - 0 1"),
    ("white +1N, white to move (expect > 0)", "1k6/pp3ppp/8/8/8/5N2/PP3PPP/1K6 w - - 0 1"),
    ("mate in 1 available (back rank)", "6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1"),
]


def human_bytes(num):
    for unit in ("B", "KB", "MB", "GB"):
        if abs(num) < 1024.0:
            return f"{num:.1f} {unit}"
        num /= 1024.0
    return f"{num:.1f} TB"


def human_count(num):
    for div, unit in ((1e9, "B"), (1e6, "M"), (1e3, "K")):
        if abs(num) >= div:
            return f"{num / div:.2f}{unit}"
    return str(num)


def section(title):
    print()
    print("=" * 78)
    print(title)
    print("=" * 78)


# ---------------------------------------------------------------------------
# state_dict loading and structure
# ---------------------------------------------------------------------------

def load_state_dict(path):
    import torch

    obj = torch.load(path, map_location="cpu", weights_only=True)
    # train.py saves a bare state_dict, but tolerate wrapped checkpoints.
    if isinstance(obj, dict) and not any(hasattr(v, "shape") for v in obj.values()):
        for key in ("state_dict", "model", "model_state_dict", "weights"):
            if key in obj and isinstance(obj[key], dict):
                return OrderedDict(obj[key]), obj
    return OrderedDict(obj), None


def is_float_tensor(tensor):
    try:
        return tensor.is_floating_point()
    except AttributeError:
        return False


def classify(name):
    """Group a state_dict key into a logical component."""
    if name.startswith("res_blocks."):
        return "trunk/res_blocks"
    if name.startswith("conv_input") or name.startswith("bn_input"):
        return "trunk/input"
    if name.startswith("policy"):
        return "head/policy"
    if name.startswith("value"):
        return "head/value"
    return "other"


def report_architecture(state_dict, path, wrapper):
    section(f"ARCHITECTURE  —  {os.path.basename(path)}")

    stat = os.stat(path)
    print(f"file                : {path}")
    print(f"file size           : {human_bytes(stat.st_size)}")
    print(f"last modified       : {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime))}")
    if wrapper:
        extra = [k for k in wrapper.keys() if not isinstance(wrapper[k], dict)]
        if extra:
            print(f"checkpoint extras   : {', '.join(map(str, extra))}")

    # Topology inference from key names / shapes.
    block_ids = set()
    for name in state_dict:
        match = re.match(r"res_blocks\.(\d+)\.", name)
        if match:
            block_ids.add(int(match.group(1)))

    channels = None
    input_planes = None
    if "conv_input.weight" in state_dict:
        shape = tuple(state_dict["conv_input.weight"].shape)
        channels, input_planes = shape[0], shape[1]

    policy_out = None
    if "policy_fc.weight" in state_dict:
        policy_out = tuple(state_dict["policy_fc.weight"].shape)[0]
    value_out = None
    if "value_fc2.weight" in state_dict:
        value_out = tuple(state_dict["value_fc2.weight"].shape)[0]

    print(f"residual blocks     : {len(block_ids)}")
    print(f"trunk channels      : {channels}")
    print(f"input planes        : {input_planes}  (expected 14: 12 piece + color + castling)")
    print(f"policy head outputs : {policy_out}  (expected 4672 = 64 squares x 73 planes)")
    print(f"value head outputs  : {value_out}  (expected 1, tanh)")

    if policy_out is not None and policy_out != 4672:
        print("  !! policy width does not match the engine's POLICY_SIZE of 4672")
    if input_planes is not None and input_planes != 14:
        print("  !! input planes do not match the engine's board encoder")

    # Parameter accounting: distinguish learnable params from BN buffers.
    params = 0
    buffers = 0
    param_bytes = 0
    for name, tensor in state_dict.items():
        count = tensor.numel()
        if name.endswith(("running_mean", "running_var", "num_batches_tracked")):
            buffers += count
        else:
            params += count
            param_bytes += count * tensor.element_size()

    print()
    print(f"learnable parameters: {params:,}  ({human_count(params)})")
    print(f"buffers (BN stats)  : {buffers:,}")
    print(f"parameter memory    : {human_bytes(param_bytes)} at fp32")

    by_component = defaultdict(int)
    for name, tensor in state_dict.items():
        if not name.endswith(("running_mean", "running_var", "num_batches_tracked")):
            by_component[classify(name)] += tensor.numel()
    print()
    print("parameters by component:")
    for comp, count in sorted(by_component.items(), key=lambda kv: -kv[1]):
        share = 100.0 * count / max(1, params)
        print(f"  {comp:<20} {count:>12,}  {share:5.1f}%")

    report_flops(state_dict, channels, len(block_ids))

    # BatchNorm num_batches_tracked is a free cumulative training counter.
    tracked = [int(t.item()) for n, t in state_dict.items() if n.endswith("num_batches_tracked")]
    if tracked:
        steps = max(tracked)
        print()
        print(f"training batches seen (BN num_batches_tracked): {steps:,}")
        print(f"  at train.py's batch_size=4096 that is ~{steps * 4096 / 1e6:.1f}M position-samples")
        if len(set(tracked)) > 1:
            print(f"  note: counters disagree (min {min(tracked)}, max {steps}) — layers added later?")


def report_flops(state_dict, channels, num_blocks):
    """Rough forward-pass cost for one 8x8 position (multiply-add = 2 FLOPs)."""
    board = 8 * 8
    flops = 0
    for name, tensor in state_dict.items():
        if not name.endswith("weight") or not is_float_tensor(tensor):
            continue
        shape = tuple(tensor.shape)
        if len(shape) == 4:  # conv: [out, in, kh, kw], padding keeps 8x8
            out_c, in_c, k_h, k_w = shape
            flops += 2 * in_c * out_c * k_h * k_w * board
        elif len(shape) == 2:  # linear
            out_f, in_f = shape
            flops += 2 * in_f * out_f
    if flops:
        print()
        print(f"forward cost / position: ~{flops / 1e6:.1f} MFLOPs "
              f"({flops / 1e9:.3f} GFLOPs)")
        print(f"  a batch of 256 costs ~{flops * 256 / 1e9:.1f} GFLOPs")


# ---------------------------------------------------------------------------
# weight statistics
# ---------------------------------------------------------------------------

def tensor_stats(tensor):
    import torch

    flat = tensor.detach().float().flatten()
    if flat.numel() == 0:
        return None
    absflat = flat.abs()
    return {
        "n": flat.numel(),
        "mean": flat.mean().item(),
        "std": flat.std().item() if flat.numel() > 1 else 0.0,
        "min": flat.min().item(),
        "max": flat.max().item(),
        "absmax": absflat.max().item(),
        "l2": torch.linalg.vector_norm(flat).item(),
        "rms": flat.pow(2).mean().sqrt().item(),
        "near_zero": (absflat < 1e-6).float().mean().item(),
    }


def report_parameters(state_dict, all_tensors):
    section("PARAMETER STATISTICS")

    rows = []
    for name, tensor in state_dict.items():
        if name.endswith("num_batches_tracked") or not is_float_tensor(tensor):
            continue
        stats = tensor_stats(tensor)
        if stats:
            rows.append((name, tuple(tensor.shape), stats))

    if not rows:
        print("no float tensors found")
        return

    header = f"{'tensor':<40}{'shape':>18}{'mean':>10}{'std':>10}{'absmax':>10}{'~0%':>7}"

    if all_tensors:
        print(header)
        print("-" * len(header))
        for name, shape, s in rows:
            print(f"{name:<40}{str(shape):>18}{s['mean']:>10.4f}{s['std']:>10.4f}"
                  f"{s['absmax']:>10.3f}{100 * s['near_zero']:>6.1f}%")
    else:
        # Collapse the repetitive residual tower, show everything else.
        print(header)
        print("-" * len(header))
        trunk_rows = [r for r in rows if r[0].startswith("res_blocks.")]
        other_rows = [r for r in rows if not r[0].startswith("res_blocks.")]
        for name, shape, s in other_rows:
            print(f"{name:<40}{str(shape):>18}{s['mean']:>10.4f}{s['std']:>10.4f}"
                  f"{s['absmax']:>10.3f}{100 * s['near_zero']:>6.1f}%")
        if trunk_rows:
            kinds = defaultdict(list)
            for name, shape, s in trunk_rows:
                kind = re.sub(r"^res_blocks\.\d+\.", "res_blocks.*.", name)
                kinds[kind].append(s)
            print()
            print("residual tower (aggregated across blocks; --all-tensors to expand):")
            for kind, stats_list in kinds.items():
                mean = sum(s["mean"] for s in stats_list) / len(stats_list)
                std = sum(s["std"] for s in stats_list) / len(stats_list)
                absmax = max(s["absmax"] for s in stats_list)
                nz = 100 * sum(s["near_zero"] for s in stats_list) / len(stats_list)
                print(f"{kind:<40}{'x' + str(len(stats_list)):>18}{mean:>10.4f}{std:>10.4f}"
                      f"{absmax:>10.3f}{nz:>6.1f}%")

    # Health flags worth surfacing regardless of verbosity.
    print()
    print("health flags:")
    flagged = False
    for name, shape, s in rows:
        # running_mean/running_var are activation statistics, not weights;
        # in a ResNet their magnitude legitimately grows with depth, so the
        # magnitude check would only produce false alarms on them.
        is_buffer = name.endswith(("running_mean", "running_var"))
        if not math.isfinite(s["mean"]) or not math.isfinite(s["absmax"]):
            print(f"  !! {name}: contains NaN or Inf")
            flagged = True
        elif not is_buffer and s["absmax"] > 20.0:
            print(f"  !  {name}: very large weight magnitude ({s['absmax']:.1f}) — possible instability")
            flagged = True
        elif s["near_zero"] > 0.9 and "num_batches" not in name:
            print(f"  !  {name}: {100 * s['near_zero']:.0f}% of values are ~0 — layer may be dead")
            flagged = True
    if not flagged:
        print("  none — no NaN/Inf, no exploded magnitudes, no dead tensors")


# ---------------------------------------------------------------------------
# batch norm health
# ---------------------------------------------------------------------------

def report_batchnorm(state_dict):
    section("BATCHNORM HEALTH  (running stats reflect training-time activations)")

    bn_names = sorted({n.rsplit(".", 1)[0] for n in state_dict if n.endswith("running_var")})
    if not bn_names:
        print("no BatchNorm buffers found")
        return

    print(f"{'layer':<34}{'gamma mean':>12}{'beta mean':>12}{'run_mean':>12}{'run_var':>12}{'dead ch':>9}")
    print("-" * 91)

    dead_total = 0
    channel_total = 0
    for base in bn_names:
        gamma = state_dict.get(f"{base}.weight")
        beta = state_dict.get(f"{base}.bias")
        run_mean = state_dict.get(f"{base}.running_mean")
        run_var = state_dict.get(f"{base}.running_var")
        if gamma is None or run_var is None:
            continue
        # A channel whose scale collapsed contributes nothing downstream.
        dead = int((gamma.detach().abs() < 1e-3).sum().item())
        dead_total += dead
        channel_total += gamma.numel()
        print(f"{base:<34}{gamma.mean().item():>12.4f}"
              f"{(beta.mean().item() if beta is not None else float('nan')):>12.4f}"
              f"{(run_mean.mean().item() if run_mean is not None else float('nan')):>12.4f}"
              f"{run_var.mean().item():>12.4f}{dead:>9}")

    print()
    print(f"collapsed channels (|gamma| < 1e-3): {dead_total} of {channel_total} "
          f"({100.0 * dead_total / max(1, channel_total):.1f}%)")
    print("interpretation:")
    print("  running_var near 0    -> activations barely vary; that layer may be inert")
    print("  running_var very high -> activations exploding; check for instability")
    print("  many collapsed gammas -> wasted capacity, the net pruned itself")


# ---------------------------------------------------------------------------
# head analysis
# ---------------------------------------------------------------------------

def effective_rank(singular_values):
    """exp(entropy of the normalized spectrum): how many directions are truly used."""
    import torch

    s = singular_values / singular_values.sum()
    s = s[s > 0]
    entropy = -(s * s.log()).sum()
    return torch.exp(entropy).item()


def report_heads(state_dict, do_svd):
    section("HEAD ANALYSIS")

    import torch

    value_w = state_dict.get("value_fc2.weight")
    value_b = state_dict.get("value_fc2.bias")
    if value_w is not None:
        print("value head (final layer -> tanh):")
        print(f"  weight L2 norm     : {torch.linalg.vector_norm(value_w.float()).item():.4f}")
        print(f"  weight std         : {value_w.float().std().item():.4f}")
        if value_b is not None:
            bias = value_b.float().item() if value_b.numel() == 1 else value_b.float().mean().item()
            print(f"  bias               : {bias:+.4f}  (tanh(bias) = {math.tanh(bias):+.4f} "
                  f"= output with zero activation)")
            if abs(math.tanh(bias)) > 0.3:
                print("  !  large resting bias — the head leans one way before seeing the board")

    policy_w = state_dict.get("policy_fc.weight")
    policy_b = state_dict.get("policy_fc.bias")
    if policy_w is not None:
        print()
        print("policy head (final layer -> logits):")
        pw = policy_w.float()
        print(f"  shape              : {tuple(pw.shape)}")
        print(f"  weight std         : {pw.std().item():.4f}")
        row_norms = torch.linalg.vector_norm(pw, dim=1)
        print(f"  per-output norm    : mean {row_norms.mean().item():.4f}, "
              f"min {row_norms.min().item():.4f}, max {row_norms.max().item():.4f}")
        unused = int((row_norms < 1e-4).sum().item())
        print(f"  near-zero outputs  : {unused} of {row_norms.numel()} "
              f"(move slots the net never activates)")
        if policy_b is not None:
            pb = policy_b.float()
            print(f"  bias               : mean {pb.mean().item():+.4f}, std {pb.std().item():.4f}, "
                  f"range [{pb.min().item():+.3f}, {pb.max().item():+.3f}]")
            print(f"  bias spread is the net's move preference before it looks at the position")

    if do_svd:
        for name in ("policy_fc.weight", "value_fc1.weight"):
            matrix = state_dict.get(name)
            if matrix is None or matrix.dim() != 2:
                continue
            print()
            started = time.time()
            try:
                sv = torch.linalg.svdvals(matrix.float())
            except Exception as exc:  # noqa: BLE001 - diagnostics must not abort
                print(f"  SVD of {name} failed: {exc}")
                continue
            total_energy = (sv ** 2).sum()
            cumulative = torch.cumsum(sv ** 2, dim=0) / total_energy
            rank99 = int((cumulative < 0.99).sum().item()) + 1
            print(f"spectrum of {name} {tuple(matrix.shape)}  ({time.time() - started:.1f}s):")
            print(f"  top singular values: {', '.join(f'{v:.2f}' for v in sv[:6].tolist())}")
            print(f"  condition number   : {(sv[0] / max(sv[-1].item(), 1e-12)):.1f}")
            print(f"  effective rank     : {effective_rank(sv):.1f} of {min(matrix.shape)}")
            print(f"  rank for 99% energy: {rank99} of {min(matrix.shape)}")
            print("  a low effective rank means the head collapsed onto few directions")


# ---------------------------------------------------------------------------
# behavioral probe
# ---------------------------------------------------------------------------

def report_behavior(path):
    # Imported before the header so a missing dep prints one clean message.
    import torch
    import chess

    sys.path.insert(0, os.path.join(REPO_ROOT, "training"))
    from model import AlphaZeroNet
    from train import ChessDataset, uci_to_index

    section("BEHAVIORAL PROBE  (network output, no search)")

    net = AlphaZeroNet()
    net.load_state_dict(torch.load(path, map_location="cpu", weights_only=True))
    net.eval()
    encoder = ChessDataset("__no_data_dir__")

    print(f"{'position':<46}{'value':>9}{'legal H':>9}{'max p':>8}  top moves")
    print("-" * 100)

    saturated = 0
    with torch.no_grad():
        for label, fen in PROBE_POSITIONS:
            board = chess.Board(fen)
            turn = fen.split(" ")[1]
            logits, value = net(encoder.fen_to_tensor(fen).unsqueeze(0))
            value = value.item()
            if abs(value) > 0.99:
                saturated += 1

            legal = [(m.uci(), logits[0][uci_to_index(m.uci(), turn)].item())
                     for m in board.legal_moves]
            if not legal:
                # Mate or stalemate: no distribution to summarize.
                print(f"{label:<46}{value:>+9.4f}{'-':>9}{'-':>8}  (no legal moves)")
                continue
            legal.sort(key=lambda x: -x[1])
            probs = torch.softmax(torch.tensor([v for _, v in legal]), dim=0)
            entropy = float(-(probs * probs.clamp_min(1e-12).log()).sum())
            top = " ".join(u for u, _ in legal[:3])
            print(f"{label:<46}{value:>+9.4f}{entropy:>9.3f}{probs.max().item():>8.3f}  {top}")

    print()
    print(f"saturated values (|v| > 0.99): {saturated} of {len(PROBE_POSITIONS)}")
    print("interpretation:")
    print("  legal H  = policy entropy over legal moves (ln(#legal) ~ 3.4 is uniform, 0 is one-hot)")
    print("  a value head stuck near 0 everywhere has not learned material;")
    print("  one pinned at +/-1 everywhere cannot rank won positions by how won they are")


# ---------------------------------------------------------------------------
# checkpoint comparison
# ---------------------------------------------------------------------------

def report_compare(paths):
    section("CHECKPOINT DRIFT  (which layers are still learning)")

    import torch

    loaded = []
    for path in paths:
        state, _ = load_state_dict(path)
        loaded.append((path, state))

    tracked_series = []
    for path, state in loaded:
        tracked = [int(t.item()) for n, t in state.items() if n.endswith("num_batches_tracked")]
        tracked_series.append(max(tracked) if tracked else None)

    print("training batches seen per checkpoint:")
    for (path, _), steps in zip(loaded, tracked_series):
        print(f"  {os.path.basename(path):<40}{'?' if steps is None else f'{steps:,}'}")

    for idx in range(1, len(loaded)):
        prev_path, prev = loaded[idx - 1][0], loaded[idx - 1][1]
        curr_path, curr = loaded[idx][0], loaded[idx][1]

        print()
        print(f"--- {os.path.basename(prev_path)}  ->  {os.path.basename(curr_path)} ---")
        steps_prev, steps_curr = tracked_series[idx - 1], tracked_series[idx]
        if steps_prev is not None and steps_curr is not None:
            print(f"    training batches elapsed: {steps_curr - steps_prev:,}")

        deltas = []
        by_component = defaultdict(lambda: [0.0, 0.0])
        for name, tensor in curr.items():
            if name not in prev or not is_float_tensor(tensor):
                continue
            if name.endswith("num_batches_tracked"):
                continue
            before = prev[name].float()
            after = tensor.float()
            if before.shape != after.shape:
                continue
            delta = torch.linalg.vector_norm(after - before).item()
            base = torch.linalg.vector_norm(before).item()
            rel = delta / base if base > 1e-12 else float("nan")
            deltas.append((name, delta, base, rel))
            comp = by_component[classify(name)]
            comp[0] += delta ** 2
            comp[1] += base ** 2

        if not deltas:
            print("    no comparable tensors")
            continue

        print(f"    {'component':<24}{'relative change':>18}")
        for comp, (dsq, bsq) in sorted(by_component.items()):
            rel = math.sqrt(dsq) / math.sqrt(bsq) if bsq > 0 else float("nan")
            print(f"    {comp:<24}{100 * rel:>17.3f}%")

        movers = sorted((d for d in deltas if math.isfinite(d[3])), key=lambda d: -d[3])[:8]
        print(f"    fastest-moving tensors:")
        for name, delta, base, rel in movers:
            print(f"      {name:<44}{100 * rel:>8.3f}%")

        total_delta = math.sqrt(sum(d[1] ** 2 for d in deltas))
        total_base = math.sqrt(sum(d[2] ** 2 for d in deltas))
        overall = 100 * total_delta / total_base if total_base > 0 else float("nan")
        print(f"    overall weight change: {overall:.3f}%")
        if overall < 0.05:
            print("      -> nearly frozen: training has converged or stalled")
        elif overall > 10:
            print("      -> very large: high learning rate or a distribution shift in the data")


# ---------------------------------------------------------------------------
# onnx
# ---------------------------------------------------------------------------

def report_onnx(path):
    import onnx  # imported before the header so a missing dep prints one message

    section(f"ONNX GRAPH  —  {os.path.basename(path)}")

    model = onnx.load(path)
    stat = os.stat(path)
    print(f"file size        : {human_bytes(stat.st_size)}")
    print(f"ir version       : {model.ir_version}")
    print(f"producer         : {model.producer_name} {model.producer_version}")
    for opset in model.opset_import:
        print(f"opset            : domain '{opset.domain or 'ai.onnx'}' version {opset.version}")

    def describe(value_info):
        dims = []
        for dim in value_info.type.tensor_type.shape.dim:
            dims.append(dim.dim_param if dim.dim_param else str(dim.dim_value))
        return f"{value_info.name}: [{', '.join(dims)}]"

    print()
    print("inputs :")
    for value_info in model.graph.input:
        print(f"  {describe(value_info)}")
    print("outputs:")
    for value_info in model.graph.output:
        print(f"  {describe(value_info)}")

    op_counts = defaultdict(int)
    for node in model.graph.node:
        op_counts[node.op_type] += 1
    print()
    print(f"nodes: {len(model.graph.node)} total")
    for op_type, count in sorted(op_counts.items(), key=lambda kv: -kv[1]):
        print(f"  {op_type:<24}{count:>5}")

    init_params = 0
    for initializer in model.graph.initializer:
        size = 1
        for dim in initializer.dims:
            size *= dim
        init_params += size
    print()
    print(f"initializer parameters: {init_params:,} ({human_count(init_params)})")

    names = {vi.name for vi in model.graph.output}
    if "policy" not in names or "value" not in names:
        print("  !! expected outputs named 'policy' and 'value' — the engine looks these up by name")


# ---------------------------------------------------------------------------
# loss history
# ---------------------------------------------------------------------------

LOSS_RE = re.compile(r"Loss:\s*([0-9]*\.?[0-9]+)")


def sparkline(values, width=70):
    if not values:
        return ""
    blocks = "▁▂▃▄▅▆▇█"
    step = max(1, len(values) / width)
    sampled = [values[min(len(values) - 1, int(i * step))] for i in range(min(width, len(values)))]
    low, high = min(sampled), max(sampled)
    span = high - low
    if span <= 0:
        return blocks[0] * len(sampled)
    return "".join(blocks[min(len(blocks) - 1, int((v - low) / span * len(blocks)))] for v in sampled)


def report_losses(path):
    section(f"LEARNING CURVE  —  {os.path.basename(path)}")

    losses = []
    with open(path, errors="replace") as handle:
        for line in handle:
            match = LOSS_RE.search(line)
            if match:
                losses.append(float(match.group(1)))

    if not losses:
        print("no 'Loss: <value>' lines found")
        return

    print(f"training cycles logged: {len(losses)}")
    print(f"first / last          : {losses[0]:.4f}  ->  {losses[-1]:.4f}")
    print(f"min / max             : {min(losses):.4f}  /  {max(losses):.4f}")

    print()
    print(f"curve (oldest -> newest, range {min(losses):.2f}..{max(losses):.2f}):")
    print("  " + sparkline(losses))

    window = max(1, len(losses) // 10)
    print()
    print(f"decile means (window of {window} cycles):")
    for start in range(0, len(losses), window):
        chunk = losses[start:start + window]
        if chunk:
            print(f"  cycles {start:>5}-{start + len(chunk) - 1:<5} mean {sum(chunk) / len(chunk):.4f}")

    tail = losses[-window:]
    prev = losses[-2 * window:-window] if len(losses) >= 2 * window else []
    if prev:
        change = sum(tail) / len(tail) - sum(prev) / len(prev)
        print()
        print(f"recent trend: {change:+.4f} over the last {window} cycles vs the {window} before")
        if abs(change) < 0.01:
            print("  -> plateaued. For self-play this is normal and NOT proof of stalled learning:")
            print("     the opponent improves alongside the net, so the data keeps getting harder.")
            print("     Judge progress with scripts/arena.py, not the loss curve.")
        elif change < 0:
            print("  -> still descending")
        else:
            print("  -> rising: check for a data distribution change (new labeling rules, purge, restart)")

    print()
    print("note: the policy loss cannot go below the entropy of its own targets")
    print("      (see data_health.py). A loss beneath that floor means degenerate targets.")


# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Thorough inspection of Fenrir .pth checkpoints, .onnx exports, and loss logs")
    parser.add_argument("checkpoints", nargs="*", help=".pth checkpoint(s); 2+ enables drift comparison")
    parser.add_argument("--onnx", help="also analyze this .onnx export")
    parser.add_argument("--train-log", help="parse the learning curve from this trainer log")
    parser.add_argument("--all-tensors", action="store_true", help="print every tensor instead of aggregating the trunk")
    parser.add_argument("--no-svd", action="store_true", help="skip singular value decomposition (faster)")
    parser.add_argument("--no-probe", action="store_true", help="skip the behavioral probe (skips torch model load)")
    args = parser.parse_args()

    if not args.checkpoints and not args.onnx and not args.train_log:
        parser.error("give at least one .pth, --onnx, or --train-log")

    def run(label, func, *func_args):
        """Diagnostics are best-effort: one failing section must not kill the report."""
        try:
            func(*func_args)
        except ImportError as exc:
            section(f"{label} — SKIPPED")
            print(f"missing dependency: {exc}")
            print("install with: pip install -r training/requirements.txt")
        except Exception as exc:  # noqa: BLE001
            section(f"{label} — FAILED")
            print(f"{type(exc).__name__}: {exc}")

    for path in args.checkpoints:
        if not os.path.exists(path):
            print(f"skipping missing file: {path}")
            continue
        try:
            state_dict, wrapper = load_state_dict(path)
        except ImportError as exc:
            print(f"cannot load {path}: {exc}\ninstall with: pip install -r training/requirements.txt")
            break
        except Exception as exc:  # noqa: BLE001
            print(f"cannot load {path}: {type(exc).__name__}: {exc}")
            continue

        run("ARCHITECTURE", report_architecture, state_dict, path, wrapper)
        run("PARAMETER STATISTICS", report_parameters, state_dict, args.all_tensors)
        run("BATCHNORM HEALTH", report_batchnorm, state_dict)
        run("HEAD ANALYSIS", report_heads, state_dict, not args.no_svd)
        if not args.no_probe:
            run("BEHAVIORAL PROBE", report_behavior, path)

    if len(args.checkpoints) >= 2:
        run("CHECKPOINT DRIFT", report_compare, args.checkpoints)

    if args.onnx:
        run("ONNX GRAPH", report_onnx, args.onnx)

    if args.train_log:
        run("LEARNING CURVE", report_losses, args.train_log)

    print()


if __name__ == "__main__":
    main()
