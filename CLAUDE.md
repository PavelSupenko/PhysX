# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A fork of NVIDIA PhysX (merged with the O3DE community fork for Mac/iOS/Android support). The **actual work of this project happens in [blast/](blast/)** — the Blast destruction SDK, unchained from NVIDIA's `packman` build system and ported to plain CMake + a Python wrapper script, so it can be built as shared libraries for mobile/desktop and consumed from Unity via P/Invoke.

`physx/` and `flow/` are the upstream NVIDIA SDKs, largely untouched; they still use `generate_projects.sh` + packman and are not part of the Blast build.

### Related working directories

| Path | Role |
|---|---|
| `PhysX/blast/` | C++ source of the Blast SDK and the custom `NvBlastExtBridge` C-API bridge (the thing being developed) |
| `../blast-unity/` | Unity 6 project; the C# side lives in `Packages/com.pavlo-supenko.unity-blaster/` |
| `../Blast/` | Pristine upstream `NVIDIAGameWorks/Blast` clone — read-only reference for original SDK behavior/samples |

Native libraries are built in `PhysX/blast/`, then **copied by hand** into `blast-unity/Packages/com.pavlo-supenko.unity-blaster/Plugins/<platform>/`. There is no automated deployment step. Each `.dylib`/`.so` needs a Unity `.meta` PluginImporter file with the right platform enabled — copy the settings from a sibling library rather than letting Unity regenerate them.

Copy the libraries, not the build directory. `macos-arm64/` had picked up CMake's generated `Makefile` along with the `UnitTests` and `TestProgram` executables — 1.5 MB of build output that Unity imported and shipped as part of the package.

Check what you built before shipping it: `lipo -info` for the architecture and `vtool -show-build` for the platform and deployment target. The `ios-arm64` slot held a **macOS** dylib for a long time without anyone noticing, because nothing in the copy step distinguishes them. The iOS build also needs its deployment target set explicitly — CMake otherwise pins `minos` to the SDK version and the plugin silently stops loading on anything older:

```bash
cmake -S . -B build_artifacts/build/ios-arm64 -G Xcode -DNV_CONFIGURATION_TYPE=release \
  -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO
```

Signing has to be off or the build stops asking for a development team; Unity signs the plugin when it builds the app.

**Delete the old library before copying, don't overwrite it in place.** If the Editor has the project open it has the `.dylib` loaded, and overwriting keeps the same inode. macOS then guards that image: `git add` and even `git hash-object` on the file die with SIGKILL, while `shasum` and `cp` read it fine, which makes the failure look like a git or sandbox problem rather than a file one. `rm` followed by `cp` gives a fresh inode and the symptom disappears.

## Building

### Blast (the primary build)

```bash
cd blast
python3 build.py          # interactive: pick target platform, Debug/Release, NDK path for Android
```

`build.py` only wraps CMake argument selection. Artifacts land in `blast/build_artifacts/build/<target>/`. The script **deletes the build directory first**, so every run is a clean configure + build; for iteration prefer driving CMake directly:

```bash
cd blast
cmake --build build_artifacts/build/macos-arm64 --config Debug -- -j8
```

Targets: `macos-{x86_64,arm64}`, `ios-arm64`, `android-{armv7,armv8,x86,x86_64}` from a macOS host; `windows-{x86,x86_64}` + Android from Windows. macOS/iOS use the Xcode generator, Android uses Unix Makefiles with the NDK toolchain file, Windows uses a Visual Studio generator.

Build logs go to `cmake_configure.log` / `cmake_build.log` inside the build directory, **not** stdout — check them when a build fails.

### Tests

`UnitTests` (GoogleTest, fetched via `FetchContent` at configure time — the first configure needs network) is built as part of the normal build:

```bash
./build_artifacts/build/macos-arm64/UnitTests
./build_artifacts/build/macos-arm64/UnitTests --gtest_filter=ActorTests.*     # single suite
```

A green run is **123 passing**, and the suite is clean under AddressSanitizer too — treat any ASan finding as a regression. Note that ASan halts on the first error, so one failure hides every later one; after fixing one, re-run before concluding anything.

`BlastBaseTest` registers itself as Blast's global error callback in its constructor and clears it in its destructor. The clear matters: gtest destroys the test object after each test, and without it any Blast message logged from *outside* a live test calls into freed memory. That produced a crash that looked like it lived in serialization and survived changing the codec, the manager's lifetime and the release order — AddressSanitizer is what found it. An ASan build can be configured with:

```bash
cmake -S . -B build_artifacts/build/macos-arm64-asan -DNV_CONFIGURATION_TYPE=release \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address"
```

`TestProgram` is a hand-edited scratch harness ([blast/source/program/TestProgram.cpp](blast/source/program/TestProgram.cpp)) that fractures a hardcoded cube through the session C-API — the fastest way to debug the bridge without launching Unity. Edit `fracture()` to select the algorithm under test. VS Code has a `Debug Blast Test Program` lldb launch config pointed at the `macos-arm64` build.

Unity-side tests live in `Packages/com.pavlo-supenko.unity-blaster/Tests/` (NUnit, Editor-only) and run from Unity's Test Runner. To run them headlessly, work on an APFS clone — an open Editor holds the project lock, and `cp -Rc` costs neither time nor disk:

```bash
cp -Rc ../blast-unity /tmp/bu && \
/Applications/Unity/Hub/Editor/6000.3.5f1/Unity.app/Contents/MacOS/Unity \
  -batchmode -runTests -projectPath /tmp/bu -testPlatform EditMode \
  -testResults /tmp/results.xml -logFile /tmp/unity.log
```

A green EditMode run is **59 passing, 2 skipped** (the two `ExportDiagnosticsTests` timing tests are `[Explicit]`). Results go to the XML file, not stdout.

### Library layout

Each `blast/cmake/NvBlast*.cmake` file defines exactly one shared library; `blast/CMakeLists.txt` includes them. Note `NvBlastExtPhysX.cmake` exists but is deliberately **not** included — the PhysX-dependent extension is not built, which is why collision hull generation is supplied locally (see below).

## Architecture: the engine bridge

The layer being developed is `NvBlastExtBridge` — a flat `extern "C"` surface over Blast's authoring API, plus a matching C# P/Invoke layer.

```
Unity Editor (BlastAuthoringWindow)
  → FractureSession.cs          holds one session for the life of the window
      → NativeMeshBuilder       Unity Mesh  → native Nv::Blast::Mesh*
      → NvBlastExtBridgeSession.cs / NvBlastExtBridge.cs   [DllImport("NvBlastExtBridge")]
          ══════════ P/Invoke boundary ══════════
      → NvBlastExtBridgeSession.cpp  C-API over the session
          → FractureSession     owns the FractureTool + RNG; per-chunk fracture ops
          → NvBlastExtAuthoringProcessFracture → AuthoringResult
      → FractureResultProcessor.ToUnityMesh   native Mesh* → UnityEngine.Mesh
```

There is one fracture path and it is the session: the caller creates a session, sets source meshes,
fractures chunks by ID, and finalizes. An earlier one-shot API built a tool, fractured and destroyed
it inside a single call — it could not subdivide, undo or preview, and it is gone.

Key files: [blast/include/extensions/bridge/NvBlastExtBridgeSession.h](blast/include/extensions/bridge/NvBlastExtBridgeSession.h) (the fracture contract) and [NvBlastExtBridge.h](blast/include/extensions/bridge/NvBlastExtBridge.h) (mesh construction, collision builder, asset serialization — what a caller needs either side of a session), their implementations under [blast/source/sdk/extensions/bridge/](blast/source/sdk/extensions/bridge/), and their C# mirrors in `blast-unity/Packages/com.pavlo-supenko.unity-blaster/Runtime/Extensions/Unity/`.

### The authoring session

`FractureSession` ([blast/source/sdk/extensions/bridge/FractureSession.h](blast/source/sdk/extensions/bridge/FractureSession.h)), exposed over C as [NvBlastExtBridgeSession.h](blast/include/extensions/bridge/NvBlastExtBridgeSession.h), owns a `FractureTool` across many operations. This is the path an authoring tool uses: fracture a chunk, inspect it, undo it, subdivide a child, finalize for preview, keep editing.

Two rules matter when extending it:

- **Chunks are addressed by chunk ID, never by info index.** IDs are stable; `getChunkInfoIndex` converts at the boundary. Info indices shift whenever chunks are added or removed, so handing one to C# guarantees a stale-reference bug.
- **Booleans cross the ABI as `NvBlastExtBridgeBool` (`uint32_t`).** C++ `bool` is 1 byte and C# `bool` marshals as a 4-byte `BOOL`; the mismatch is silent.

The generator is re-seeded before every operation, so a fracture depends only on its seed and parameters, never on how many operations preceded it. That is what makes a stored seed reproduce an asset — and what the authoring recipe on the Unity side is built on.

### Support graph and anchoring

Which chunks are support chunks follows the depth rule passed to Finalize. What that rule cannot express is anchoring: with nothing bonding the asset to the world, a structure has no support and collapses on the first simulated frame.

Chunks can be marked static; Finalize gives each one a bond to the external body via `NvBlastExtAssetUtilsAddExternalBonds`. Marks are held as **chunk IDs** and translated through `assetToFractureChunkIdMap` at finalize time, because finalizing reorders chunks and the asset's indices do not match the session's IDs.

Only support chunks can carry an external bond. A chunk marked static that the depth rule did not make support is reported and skipped — `NvBlastExtBridgeSessionIsChunkSupport` mirrors the rule so a tool can check first.

Arbitrary per-chunk support overrides are deliberately absent: changing which chunks are upper-support changes the chunk ordering `NvBlastCreateAsset` requires, so honouring them means duplicating the whole `ProcessFracture` pipeline rather than post-processing its result.

### Asset serialization

The `NvBlastAsset` is freed with its `AuthoringResult`, so without serializing it the entire authoring outcome is discarded moments after being produced. `NvBlastExtBridgeSerializeAsset` / `DeserializeAsset` / `ReleaseSerializedAsset` in [NvBlastExtBridge.h](blast/include/extensions/bridge/NvBlastExtBridge.h) copy it out; this is what the runtime will load.

The serialization manager is created per process, never released. `NvBlastExtLlSerializerLoadSet` also registers the family codecs, which need the Tk framework this extension does not build — those two fail and log once. That complaint is noise, but it once mattered: it was the message that exposed a use-after-free in the test harness (see below).

### Adding a fracture algorithm

Touch these in order:
1. `blast/include/extensions/bridge/NvBlastExtBridgeConfigs.h` — its config struct, if it needs one that Blast's authoring headers don't already provide (`SlicingConfiguration` and friends come from `NvBlastExtAuthoringFractureTool.h`). A config with more than a couple of fields, or one carrying a bitmap, belongs in `NvBlastExtBridgeSession.h` next to `NvBlastExtBridgeCutoutConfiguration` instead — that file is where the flattened-for-ABI parameter sets live.
2. `FractureSession.h` / `.cpp` — a `fracture<Name>` method.
3. `NvBlastExtBridgeSession.h` / `.cpp` — the session entry point.
4. C# `DllImport`s, a config `struct` under `Runtime/Extensions/Unity/Configs/` or `Session/`, and a `FractureType` enum entry.
5. `Editor/Windows/BlastAuthoringWindow.cs` — the settings GUI case, and a `RecordFractureStep` case so the operation lands in the authoring recipe.

Tests live in [blast/source/test/src/unit/FractureSessionTests.cpp](blast/source/test/src/unit/FractureSessionTests.cpp) and exercise the C API rather than the C++ class, because the C API is the contract engine integrations bind to.

Config structs cross the boundary **by value**. The C# side must be `[StructLayout(LayoutKind.Sequential)]` with field types and order matching the C++ struct exactly, including nested structs like `NoiseConfiguration`. A mismatch produces garbage parameters, not an error.

### Native memory ownership

The bridge hands raw pointers to C#, so ownership rules are conventions, not enforced. Getting them wrong crashes the Editor. The current contract:

- `NvBlastExtBridgeCleanMesh` **releases the mesh it is given** and returns a new one. The C# side calls `NativeMeshHandle.DetachPointer()` before the call so the old handle never double-releases.
- `NvBlastExtBridgeCreateMeshes` returns a `Mesh**`. Release each `Mesh*` individually, *then* `NvBlastExtBridgeReleaseMeshesArray` for the array itself.
- The `ConvexMeshBuilder` must outlive the `AuthoringResult` — call `NvBlastExtBridgeReleaseAuthoringResult(builder, result)` first, `NvBlastExtBridgeReleaseCollisionBuilder(builder)` second. Builder creation/release was deliberately split out of the fracture call so the Unity side controls this ordering.
- C# wrappers (`NativeMeshHandle`, `SafePointer`, `FractureSession`) are `IDisposable` with finalizers. The session owns the native session handle and is disposed when the Editor window closes.

### `ConvexHullMeshBuilder`

Blast's own `ConvexMeshBuilder` lives in `NvBlastExtPhysX`, which this project doesn't build, so [blast/source/sdk/extensions/bridge/ConvexHullMeshBuilder.cpp](blast/source/sdk/extensions/bridge/ConvexHullMeshBuilder.cpp) supplies a replacement with no PhysX dependency. It computes real hulls with `btConvexHullComputer` — the quickhull already compiled into `NvBlastExtAuthoring` as part of V-HACD, so it costs no new dependency, only the `VHACD/inc` include path in [cmake/NvBlastExtBridge.cmake](blast/cmake/NvBlastExtBridge.cmake).

Two details worth keeping in mind when touching it:

- Face planes are fitted through the face centroid, not through a picked vertex. Hull vertices are single-precision, so a quad or larger face is never exactly planar; fitting through the centroid spreads the residual instead of pinning it to one vertex. Expect a few 1e-4 of slack at unit scale — the tests allow for it.
- Normals are computed with Newell's method and then oriented against the hull centroid. Hull faces are n-gons whose consecutive vertices are often nearly collinear, which makes a cross product of three picked vertices unstable.

Degenerate input — fewer than four points, or points that are collinear or coplanar — has no volume and no hull, so it falls back to a bounding box rather than dropping the chunk's collision.

An earlier `BoundingBoxConvexMeshBuilder` returned a box for *every* chunk; it is gone. The Unity side reads these hulls through `HullMeshConverter` and turns each into a collider mesh, so a chunk decomposed into several hulls gets one collider per hull instead of a single convex hull of the whole piece.

## The Unity authoring tool

`BlastAuthoringWindow` (Tools → Blast Authoring) drives a session interactively. Three pieces of it are worth knowing before changing anything:

- **Chunks are drawn by hidden proxy objects** (`ChunkPreviewRenderer`), not `Graphics.DrawMesh`. That call submits a mesh for one frame and the Scene View only repaints on interaction, so chunks flickered and vanished; forcing a repaint every tick fixes that but keeps the editor redrawing all day. Proxies carry `HideAndDontSave`, so they never reach the Hierarchy, a saved scene, or the undo stack.
- **The Scene View is the viewport**, deliberately — it already provides camera, lighting, materials and is the only place overlays can be drawn over the chunks. Picking is analytic (Möller–Trumbore against cached triangles).
- **Explode is per chunk and compounds down the hierarchy.** Each step is measured against its own parent's centre and radius, so a piece spreads from the piece it was cut from. Picking shifts the ray by the same accumulated offset, or clicks would miss exactly when the view is open.

Two asset types come out of it, in [Runtime/Assets/](../blast-unity/Packages/com.pavlo-supenko.unity-blaster/Runtime/Assets/):

- `BlastFractureAsset` — the build output: serialized Blast asset, chunk meshes, hull meshes, stress defaults.
- `BlastAuthoringRecipe` — the source: the steps that produced the hierarchy, replayable because fracturing is deterministic in its seed. Stores steps, not geometry, so it stays a few kilobytes and stays meaningful when the algorithms improve.

`BlastAssetExporter` writes them. Two things there are load-bearing: mesh writes are wrapped in `AssetDatabase.StartAssetEditing`/`StopAssetEditing` (without it each `CreateAsset` imports immediately, and a few hundred chunks take minutes that look like a freeze), and mesh paths are fixed rather than unique (`GenerateUniqueAssetPath` left a full extra copy of every mesh on each re-export).

## Conventions

- C++14, 4-space indent, NVIDIA `Nv`/`NvBlast` naming. Use the `NVBLASTLL_LOG_DEBUG/ERROR(logFn, …)` macros with the `NvBlastLog` callback passed in from C# rather than `printf` — that callback routes into the Unity console via `BlastLogger`.
- Every exported function is prefixed `NvBlastExtBridge` and declared `NV_C_API`; the C# `DllName` is `"NvBlastExtBridge"` (Unity resolves the platform prefix/extension).
- The `develop` branch is the integration branch; feature branches use `feature/unity/<topic>`. `main` tracks upstream.
