#!/usr/bin/env python3
"""Convert a Fenrir .pth checkpoint to the .onnx format the engine loads.

Usage: python3 scripts/export_onnx.py checkpoints/fenrir_20260718_0400.pth out.onnx
"""

import sys
import os

import torch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
from model import AlphaZeroNet  # noqa: E402


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    pth_path, onnx_path = sys.argv[1], sys.argv[2]

    model = AlphaZeroNet()
    model.load_state_dict(torch.load(pth_path, map_location="cpu", weights_only=True))
    model.eval()

    torch.onnx.export(
        model,
        torch.randn(1, 14, 8, 8),
        onnx_path,
        export_params=True,
        opset_version=14,
        input_names=["input"],
        output_names=["policy", "value"],
        dynamic_axes={"input": {0: "batch_size"},
                      "policy": {0: "batch_size"},
                      "value": {0: "batch_size"}},
    )
    print(f"exported {pth_path} -> {onnx_path}")


if __name__ == "__main__":
    main()
