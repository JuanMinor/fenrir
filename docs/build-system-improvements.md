# Build System Improvements (planning notes)

Status: **not yet implemented** — captured for discussion, revisit before coding.

## Background

Found while trying to show a friend progress on Fenrir: the build failed
because their compiler was older than GCC 13, which doesn't have `<format>`
(needed for C++20). Separately, `CMakeLists.txt` has no awareness of GPU
vendor (NVIDIA vs AMD) on either Windows or Linux, and pins a specific
ONNX Runtime version that will eventually drift from what's actually
available upstream.

## 1. Compiler version gate

Add a check right after `project()` in `CMakeLists.txt` that hard-fails
configure (not just warns) if the compiler is too old for `<format>`:

- GCC: require >= 13
- Clang: require >= 17
- MSVC: require >= 19.29 (VS 2019 16.11 / VS 2022)

Fail with `message(FATAL_ERROR ...)` naming the actual detected compiler
and version, and what's required, so the failure is immediate and
legible instead of a cryptic `<format>: No such file or directory` deep
in a compile log.

## 2. Stop pinning the ONNX Runtime version

Today `ONNXRUNTIME_VERSION` is hardcoded (`1.20.1`) and the download URL
is constructed by string-formatting that version into an assumed
filename pattern. Two problems found while investigating:

- **Asset naming already changed upstream.** 1.20.1 published
  `onnxruntime-linux-x64-gpu-{version}.tgz`. The current latest (checked
  live: v1.27.1) instead publishes
  `onnxruntime-linux-x64-gpu_cuda12-{version}.tgz` and
  `..._cuda13-{version}.tgz` — split by CUDA major version, no plain
  `-gpu-` asset anymore. A hardcoded filename pattern silently 404s (or
  worse, downloads the wrong thing) whenever this happens again.
- **Version pin means the project drifts from upstream indefinitely**
  unless someone remembers to bump it by hand.

Plan: at configure time, query the GitHub releases API
(`https://api.github.com/repos/microsoft/onnxruntime/releases/latest`)
for the latest tag *and* its actual asset list, then **select the
download URL by pattern-matching the real returned filenames** (e.g.
regex for `linux-x64-gpu_cuda`) rather than constructing a filename
string from the version number ourselves. If nothing matches the
expected pattern, `FATAL_ERROR` loudly instead of silently mis-downloading.

Open question: CUDA12 vs CUDA13 asset — default to picking the newest
available, or match the machine's installed CUDA toolkit version?
Leaning toward "newest available" for simplicity, revisit if it causes
problems.

Windows/DirectML comes from NuGet, not GitHub releases, so it needs the
equivalent treatment via the NuGet v3 API (query latest package version
instead of hardcoding one).

## 3. GPU vendor / platform detection (Linux)

At configure time:

- **Detect WSL**: check `/proc/version` for `microsoft`/`WSL`
  (case-insensitive).
- **Detect GPU vendor**: `lspci | grep -iE 'vga|3d controller'` — chosen
  over relying on `nvidia-smi`/`rocm-smi` because it doesn't require the
  vendor's own tooling/driver to already be installed to detect the
  hardware.

Behavior by result:

| Platform      | GPU detected | Action |
|---------------|---------------|--------|
| Linux (native)| NVIDIA        | fetch CUDA onnxruntime asset (per §2) |
| Linux (native)| none          | fetch CPU-only onnxruntime asset |
| Linux (native)| AMD           | **`FATAL_ERROR`** — see below |
| WSL           | AMD           | **`FATAL_ERROR`** — different message, see below |
| Windows       | NVIDIA or AMD | unchanged: DirectML via NuGet already handles both vendors |

**Why AMD-on-native-Linux fails instead of falling back to CPU:**
confirmed (checked actual GitHub release assets, current and historical)
that Microsoft has never published a prebuilt ROCm-enabled onnxruntime
Linux binary. A user having ROCm's driver/SDK installed locally (which
*is* a normal native-Linux system package, not something to download)
doesn't change this — the onnxruntime *library itself* still needs to
have been compiled with `--use_rocm`, and no such prebuilt exists to
fetch. Building onnxruntime from source with ROCm is real but heavy
(ROCm SDK dependency, long build time) and out of scope for this pass.
Failing loudly at configure time beats today's behavior, where
`nn.cpp` silently catches the CUDA-provider failure and falls back to
CPU with just a log line — someone could be running CPU-only for weeks
without noticing.

**Why WSL+AMD gets its own message, not the native-Linux one:** the
reason is different (AMD GPU compute passthrough isn't meaningfully
supported under WSL2 at all for this build path, independent of the
ROCm-binary-availability issue on native Linux) and the fix a user would
reach for differs too (e.g. dual-boot or run natively rather than "wait
for ROCm binaries" or "build from source").

**Files touched:** `CMakeLists.txt` only. No `nn.cpp`/`hardware.cpp`
changes needed for this pass, since we're refusing to configure rather
than adding a new ROCm execution-provider code path at runtime.

---

## Resolved: WSL2 <-> Arena bridging — decided against, going native instead

Originally raised as "run the WSL2-built engine from Arena on Windows
11 via a wrapper." Talked through two options:

- **Wrapper**: thin Windows exe that Arena launches, which shells out to
  `wsl.exe -d <distro> --cd /fenrir -- ./bin/fenrir` and forwards
  stdin/stdout. Rejected — UCI is a latency-sensitive stdio protocol
  (Arena expects prompt `readyok`/`bestmove` responses), and routing it
  through an extra `wsl.exe` hop risks buffering/timing issues that
  would only show up under Arena's actual polling behavior, not a quick
  manual test. Also needs Windows<->WSL path translation for
  `fenrir.cfg`/`onnx/`.
- **Native Windows build** (chosen): the CMake Windows/DirectML path
  already exists and is vendor-agnostic (NVIDIA or AMD), so it sidesteps
  the GPU-vendor question too. Arena launches `fenrir.exe` directly, no
  bridging layer at all.

Then the actual distribution question came up: **no target Windows
machine running Arena can be assumed to have any dev tooling installed**
(no MSVC, no toolchain, nothing). So this isn't "build on the Arena
machine" — it's standard "build once, ship a self-contained binary"
distribution, same as how Stockfish or any other engine is distributed.

What that requires, concretely:

- **Statically link the MSVC C++ runtime** (`/MT` instead of the
  CMake/MSVC default `/MD`) so the exe doesn't depend on the target
  machine having the Visual C++ Redistributable installed. This is the
  one CMake-level change needed for this thread — currently
  `CMakeLists.txt` doesn't set a runtime-linking mode for MSVC at all,
  so it's on the implicit dynamic default.
- **`onnxruntime.dll` and the `onnx/` model folder still have to ship
  alongside the exe** — those aren't statically linkable (DLL-only
  distribution from Microsoft/NuGet), but `CMakeLists.txt` already
  copies both next to the binary post-build
  ([CMakeLists.txt:158-168](../CMakeLists.txt#L158-L168)), so no new
  work there beyond what already exists.
- End result: build once (on any Windows machine with a toolchain, even
  a throwaway/CI one), zip `fenrir.exe` + `onnxruntime.dll` + `onnx/` +
  `fenrir.cfg`, copy that folder to the Arena machine, point Arena at
  the exe. No installer, no prerequisites, no WSL involved at all.

Resolved: the "no tooling on the *build* machine either" concern
doesn't apply — the toolchain already exists on the user's own Windows
machine, so the one-time build just happens there. The `/MT`
static-runtime change plus zipping `fenrir.exe` + `onnxruntime.dll` +
`onnx/` + `fenrir.cfg` is the only remaining work for this thread; no
cross-compiling or CI runner needed for now. (A CI-built release
artifact remains a reasonable future nice-to-have if this needs to be
handed to other people repeatedly, but it's not blocking anything
today.)
