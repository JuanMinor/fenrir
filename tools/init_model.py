#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
Create a fresh, randomly-initialized Fenrir network and export it
straight to ONNX -- the one bootstrap step nothing else in this repo can
do on its own.

Both the engine's UCI mode and self-play need onnx/fenrir.onnx to exist
before they can run at all. export_onnx.py can only convert an
*existing* .pth checkpoint, and train.py only exports a new ONNX after
it has already consumed self-play data -- which itself needs
onnx/fenrir.onnx to be generated. That's a chicken-and-egg loop; this
script is the one thing that breaks it, by exporting straight from a
freshly-constructed, never-trained model instead of a checkpoint.

The resulting network knows nothing about chess yet (near-uniform
policy, value near 0 everywhere) -- it plays legal moves but weakly
until self-play and training have had a chance to run. Run this exactly
once on a fresh clone, before anything else that touches onnx/fenrir.onnx.

Usage:
  python3 tools/init_model.py                      # writes onnx/fenrir.onnx and onnx/fenrir.pth
  python3 tools/init_model.py --seed 1
  python3 tools/init_model.py --out /tmp/scratch.onnx --pth ""   # skip writing a .pth
"""

import argparse
import os
import sys

import torch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
from model import AlphaZeroNet  # noqa: E402


def main():
    parser = argparse.ArgumentParser(
        description="Bootstrap a fresh, randomly-initialized network and export it to ONNX",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--out", default="onnx/fenrir.onnx", help="ONNX output path (default: onnx/fenrir.onnx)")
    parser.add_argument("--pth", default="onnx/fenrir.pth",
                         help="also write the matching .pth here, so train.py resumes from these exact "
                              "weights instead of initializing its own fresh (different) random ones on "
                              "its first run; pass an empty string to skip")
    parser.add_argument("--value-channels", type=int, default=3, help="value head width (default: 3, matches train.py)")
    parser.add_argument("--seed", type=int, default=None, help="torch seed, for a reproducible init")
    args = parser.parse_args()

    if args.seed is not None:
        torch.manual_seed(args.seed)

    model = AlphaZeroNet(value_channels=args.value_channels)
    model.eval()

    if args.pth:
        os.makedirs(os.path.dirname(args.pth) or ".", exist_ok=True)
        torch.save(model.state_dict(), args.pth)
        print(f"wrote random-initialized weights to {args.pth}")

    os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
    torch.onnx.export(
        model,
        torch.randn(1, 14, 8, 8),
        args.out,
        export_params=True,
        opset_version=14,
        input_names=["input"],
        output_names=["policy", "value"],
        dynamic_axes={"input": {0: "batch_size"},
                      "policy": {0: "batch_size"},
                      "value": {0: "batch_size"}},
    )

    # Same external-data consolidation export_onnx.py does -- the engine
    # loads the .onnx as an in-memory buffer with no file path to resolve
    # external-data references against, so a side-car .onnx.data (which
    # PyTorch's exporter can write by default) would silently produce an
    # unloadable model.
    import onnx

    model_proto = onnx.load(args.out)
    onnx.save_model(model_proto, args.out, save_as_external_data=False)
    sidecar = args.out + ".data"
    if os.path.exists(sidecar):
        os.remove(sidecar)

    size_mb = os.path.getsize(args.out) / (1024 * 1024)
    print(f"wrote random-initialized network to {args.out} ({size_mb:.1f} MB, self-contained)")
    print("this network knows nothing about chess yet -- run self-play + training to improve it")


if __name__ == "__main__":
    main()
