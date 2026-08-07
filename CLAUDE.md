# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A fork of NVIDIA PhysX (merged with the O3DE community fork for Mac/iOS/Android support). The **actual work of this project happens in [blast/](blast/)** — the Blast destruction SDK, unchained from NVIDIA's `packman` build system and ported to plain CMake + a Python wrapper script, so it can be built as shared libraries for mobile/desktop and consumed from Unity via P/Invoke.

`physx/` and `flow/` are the upstream NVIDIA SDKs, largely untouched; they still use `generate_projects.sh` + packman and are not part of the Blast build.

### Related working directories

| Path | Role |
|---|---|
| `PhysX/blast/` | C++ source of the Blast SDK and the custom `NvBlastExtUnity` C-API bridge (the thing being developed) |
| `../blast-unity/` | Unity 6 project; the C# side lives in `Packages/com.pavlo-supenko.unity-blaster/` |
| `../Blast/` | Pristine upstream `NVIDIAGameWorks/Blast` clone — read-only reference for original SDK behavior/samples |

Native libraries are built in `PhysX/blast/`, then **copied by hand** into `blast-unity/Packages/com.pavlo-supenko.unity-blaster/Plugins/<platform>/`. There is no automated deployment step. Each `.dylib`/`.so` needs a Unity `.meta` PluginImporter file with the right platform enabled — copy the settings from a sibling library rather than letting Unity regenerate them.

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

`TestProgram` is a hand-edited scratch harness ([blast/source/program/TestProgram.cpp](blast/source/program/TestProgram.cpp)) that fractures a hardcoded cube through the Unity C-API — the fastest way to debug the bridge without launching Unity. Edit `createFracturer()` to select the algorithm under test. VS Code has a `Debug Blast Test Program` lldb launch config pointed at the `macos-arm64` build.

Unity-side tests live in `Packages/com.pavlo-supenko.unity-blaster/Tests/` (NUnit, Editor-only) and run from Unity's Test Runner.

### Library layout

Each `blast/cmake/NvBlast*.cmake` file defines exactly one shared library; `blast/CMakeLists.txt` includes them. Note `NvBlastExtPhysX.cmake` exists but is deliberately **not** included — the PhysX-dependent extension is not built, which is why collision hull generation is supplied locally (see below).

## Architecture: the Unity bridge

The layer being developed is `NvBlastExtUnity` — a flat `extern "C"` surface over Blast's authoring API, plus a matching C# P/Invoke layer.

```
Unity Editor (FractureWindow)
  → FracturingAsset            orchestrates Create → Clear → Fracture → SpawnGameObjects
      → NativeMeshBuilder      Unity Mesh  → native Nv::Blast::Mesh*
      → NvBlastExtUnity.cs     [DllImport("NvBlastExtUnity")]
          ══════════ P/Invoke boundary ══════════
      → NvBlastExtUnity.cpp    C-API; one-shot wrapper over a throwaway session
          → FractureSession    owns the FractureTool + RNG; per-chunk fracture ops
          → NvBlastExtAuthoringProcessFracture → AuthoringResult
      → FractureResultProcessor AuthoringResult → List<UnityEngine.Mesh>, then frees everything
```

For interactive authoring the C# side should hold a session directly rather than call the one-shot
path — see "The authoring session" below. The one-shot call re-fractures from the source mesh every
time, so it cannot subdivide or undo.

Key files: [blast/include/extensions/unity/NvBlastExtUnity.h](blast/include/extensions/unity/NvBlastExtUnity.h) (the contract), [blast/source/sdk/extensions/unity/NvBlastExtUnity.cpp](blast/source/sdk/extensions/unity/NvBlastExtUnity.cpp), and their C# mirrors in `blast-unity/Packages/com.pavlo-supenko.unity-blaster/Runtime/Extensions/Unity/`.

### The authoring session

`FractureSession` ([blast/source/sdk/extensions/unity/FractureSession.h](blast/source/sdk/extensions/unity/FractureSession.h)), exposed over C as [NvBlastExtUnitySession.h](blast/include/extensions/unity/NvBlastExtUnitySession.h), owns a `FractureTool` across many operations. This is the path an authoring tool uses: fracture a chunk, inspect it, undo it, subdivide a child, finalize for preview, keep editing.

Two rules matter when extending it:

- **Chunks are addressed by chunk ID, never by info index.** IDs are stable; `getChunkInfoIndex` converts at the boundary. Info indices shift whenever chunks are added or removed, so handing one to C# guarantees a stale-reference bug.
- **Booleans cross the ABI as `NvBlastExtUnityBool` (`uint32_t`).** C++ `bool` is 1 byte and C# `bool` marshals as a 4-byte `BOOL`; the mismatch is silent.

The generator is re-seeded before every operation, so a fracture depends only on its seed and parameters, never on how many operations preceded it. That is what makes a stored seed reproduce an asset.

### The `Fracturer` descriptor

`Nv::Blast::Fracturer` ([blast/include/extensions/unity/NvBlastFracturer.h](blast/include/extensions/unity/NvBlastFracturer.h)) is a project-specific type, not upstream Blast. It records *which* operation to run and with what settings; `FractureSession::applyFracturer` performs it. It used to be an abstract strategy that drove a `FractureTool` passed as an argument — that shape only fits the one-shot pipeline and cannot target a session's long-lived tool.

`NvBlastExtUnityFractureMeshes` is now a thin convenience wrapper that drives a throwaway session, so both APIs share one implementation.

Adding a new algorithm means touching, in order:
1. `blast/include/extensions/unity/NvBlastExtUnityConfigs.h` — its config struct, if it needs one that Blast's authoring headers don't already provide (`SlicingConfiguration` and friends come from `NvBlastExtAuthoringFractureTool.h`).
2. `FractureSession.h` / `.cpp` — a `fracture<Name>` method plus a `Fracturer::Type` case in `applyFracturer`.
3. `NvBlastExtUnitySession.h` / `.cpp` — the session entry point; and `NvBlastExtUnity.h` / `.cpp` for a `NvBlastExtUnityCreate<Name>Fracturer` factory if the one-shot path should offer it too.
4. C# `DllImport`s, a config `struct` under `Runtime/Extensions/Unity/Configs/`, and a `FractureType` enum entry.
5. `Editor/Windows/FractureWindow.cs` — the settings GUI case.

Tests live in [blast/source/test/src/unit/FractureSessionTests.cpp](blast/source/test/src/unit/FractureSessionTests.cpp) and exercise the C API rather than the C++ class, because the C API is the contract engine integrations bind to.

Config structs cross the boundary **by value**. The C# side must be `[StructLayout(LayoutKind.Sequential)]` with field types and order matching the C++ struct exactly, including nested structs like `NoiseConfiguration`. A mismatch produces garbage parameters, not an error.

### Native memory ownership

The bridge hands raw pointers to C#, so ownership rules are conventions, not enforced. Getting them wrong crashes the Editor. The current contract:

- `NvBlastExtUnityCleanMesh` **releases the mesh it is given** and returns a new one. The C# side calls `NativeMeshHandle.DetachPointer()` before the call so the old handle never double-releases.
- `NvBlastExtUnityCreateMeshes` returns a `Mesh**`. Release each `Mesh*` individually, *then* `NvBlastExtUnityReleaseMeshesArray` for the array itself.
- The `ConvexMeshBuilder` must outlive the `AuthoringResult` — call `NvBlastExtUnityReleaseAuthoringResult(builder, result)` first, `NvBlastExtUnityReleaseCollisionBuilder(builder)` second. Builder creation/release was deliberately split out of the fracture call so the Unity side controls this ordering.
- C# wrappers (`NativeMeshHandle`, `SafePointer`, `IFracturer`) are `IDisposable` with finalizers; `NativeMeshCache` keeps extracted source meshes alive across repeated fractures and is disposed when the Editor window closes.

### `ConvexHullMeshBuilder`

Blast's own `ConvexMeshBuilder` lives in `NvBlastExtPhysX`, which this project doesn't build, so [blast/source/sdk/extensions/unity/ConvexHullMeshBuilder.cpp](blast/source/sdk/extensions/unity/ConvexHullMeshBuilder.cpp) supplies a replacement with no PhysX dependency. It computes real hulls with `btConvexHullComputer` — the quickhull already compiled into `NvBlastExtAuthoring` as part of V-HACD, so it costs no new dependency, only the `VHACD/inc` include path in [cmake/NvBlastExtUnity.cmake](blast/cmake/NvBlastExtUnity.cmake).

Two details worth keeping in mind when touching it:

- Face planes are fitted through the face centroid, not through a picked vertex. Hull vertices are single-precision, so a quad or larger face is never exactly planar; fitting through the centroid spreads the residual instead of pinning it to one vertex. Expect a few 1e-4 of slack at unit scale — the tests allow for it.
- Normals are computed with Newell's method and then oriented against the hull centroid. Hull faces are n-gons whose consecutive vertices are often nearly collinear, which makes a cross product of three picked vertices unstable.

Degenerate input — fewer than four points, or points that are collinear or coplanar — has no volume and no hull, so it falls back to a bounding box rather than dropping the chunk's collision.

An earlier `BoundingBoxConvexMeshBuilder` returned a box for *every* chunk; it is gone. The Unity side still uses `MeshCollider` with `convex = true` rather than the generated hulls, so the improvement is not yet visible in-engine — wiring the hulls through is outstanding work.

## Conventions

- C++14, 4-space indent, NVIDIA `Nv`/`NvBlast` naming. Use the `NVBLASTLL_LOG_DEBUG/ERROR(logFn, …)` macros with the `NvBlastLog` callback passed in from C# rather than `printf` — that callback routes into the Unity console via `BlastLogger`.
- Every exported function is prefixed `NvBlastExtUnity` and declared `NV_C_API`; the C# `DllName` is `"NvBlastExtUnity"` (Unity resolves the platform prefix/extension).
- The `develop` branch is the integration branch; feature branches use `feature/unity/<topic>`. `main` tracks upstream.
