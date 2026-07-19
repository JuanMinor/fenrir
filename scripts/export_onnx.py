#!/usr/bin/env python3
"""Convert a Fenrir .pth checkpoint to the .onnx format the engine loads.

Usage: python3 scripts/export_onnx.py checkpoints/fenrir_20260718_0400.pth out.onnx
"""

import sys
import os

import torch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
from model import AlphaZeroNet, infer_value_channels  # noqa: E402


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    pth_path, onnx_path = sys.argv[1], sys.argv[2]

    # Match whatever value-head width this checkpoint was trained with, so
    # checkpoints from before and after a head migration both export.
    state = torch.load(pth_path, map_location="cpu", weights_only=True)
    model = AlphaZeroNet(value_channels=infer_value_channels(state))
    model.load_state_dict(state)
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

    # The engine loads the .onnx into a memory buffer and builds the session
    # from bytes, so ONNX Runtime cannot resolve external-data references
    # (it has no file path to resolve them against). PyTorch >= 2.9's
    # exporter writes weights to a side-car .onnx.data by default, which
    # would silently produce an unloadable model — consolidate into one
    # self-contained file and remove any side-car.
    import onnx

    model_proto = onnx.load(onnx_path)  # pulls external data into memory
    onnx.save_model(model_proto, onnx_path, save_as_external_data=False)
    sidecar = onnx_path + ".data"
    if os.path.exists(sidecar):
        os.remove(sidecar)

    size_mb = os.path.getsize(onnx_path) / (1024 * 1024)
    if size_mb < 10:
        sys.exit(f"ERROR: {onnx_path} is only {size_mb:.1f} MB — weights are missing")
    print(f"exported {pth_path} -> {onnx_path} ({size_mb:.1f} MB, self-contained)")


if __name__ == "__main__":
    main()
