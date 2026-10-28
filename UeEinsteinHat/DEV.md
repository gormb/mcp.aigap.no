# UeEinsteinHat — Einstein Hat Tiler Development Notes

**The topological aperiodic monotile** — "a-periodic mono-tile".

Gorm Braarvig · Aigap.no · 2026

Unreal Engine 5.8 project framework. Copied from the `UeBlank` template (blank level, MCP tooling enabled).

---

## 1. Overview

Develop a tile component to be used to emulate the random nature of reality with "material" or infinite-zoom patterns — using Hat and Penrose tiles.

The deliverable is a reusable, commercially distributable tiling system that renders aperiodic monotile patterns in real time, driven by a single seed value, and exposed both as a JavaScript reference implementation and as an Unreal Engine component.

---

## 2. Project Layout & Environment

| Path | Purpose |
| --- | --- |
| `UeEinsteinHat.uproject` | Project descriptor (Engine 5.8, `ModelContextProtocol` + `AllToolsets` plugins enabled) |
| `Config/` | Engine and project configuration (`DefaultEngine.ini`, `DefaultGame.ini`, `DefaultInput.ini`, `DefaultEditorPerProjectUserSettings.ini`) |
| `Content/` | Game assets — currently `Maps/HelloWorld` (default startup map) |
| `Source/EinsteinHatTiler/` | Runtime C++ module — the tile geometry and the two tile actors |
| `.vscode/mcp.json` | Points the editor MCP tooling at `http://127.0.0.1:8000/mcp` |
| `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `Deliveries/` | Generated directories (git-ignored, recreated on build or deploy) |
| `.vscode/` | Build tasks, launch configs and MCP server config — see §8 |
| `Scripts/GenerateProjectFiles.cmd` | Regenerates the Visual Studio 2026 solution and IntelliSense data (§8) |

### Source tree
| Path | Purpose |
| --- | --- |
| `Source/EinsteinHatTiler/Public/EinsteinHatTypes.h` | Header-only geometry "DNA": lattice points, the unit kite, the 13-vertex outline, the 8-kite decomposition, and the triangle-fan builder |
| `Source/EinsteinHatTiler/Public/EinsteinHatLibrary.h` + `Private/EinsteinHatLibrary.cpp` | `UEinsteinHatLibrary` — Blueprint access to the geometry and mesh triangulation |
| `Source/EinsteinHatTiler/Public/KiteActor.h` + `Private/KiteActor.cpp` | `AKiteActor` — the smallest physical unit (one kite) |
| `Source/EinsteinHatTiler/Public/HatShape.h` + `Private/HatShape.cpp` | `AHatShape` — the container actor (the whole hat) |
| `Scripts/Deploy/` | `deploy.ps1` manifest table plus `deployA.cmd`–`deployR.cmd`, one packaging script per delivery (§7) |

### Getting Started
1. Open `UeEinsteinHat.uproject` with Unreal Engine 5.8 (generates missing intermediate binaries/cache).
2. The editor exposes MCP tools at `http://127.0.0.1:8000/mcp`, configured via `.vscode/mcp.json`.
3. The C++ module ships with the project. If you change it, close the editor and rebuild — Live Coding cannot add a module:
   ```
   "<UE_5.8>\Engine\Build\BatchFiles\Build.bat" UeEinsteinHatEditor Win64 Development -Project="<repo>\UeEinsteinHat\UeEinsteinHat.uproject"
   ```
   Visual Studio and VS Code are wired up too — see §8.

### Planned additions
| Path | Purpose |
| --- | --- |
| `Content/Blueprints/` | `BPC_EinsteinHatTiler*` Blueprint containers |
| `Content/Materials/` | `M_HatTile*` tile and Penrose rhombus materials |
| `Scripts/` | Editor automation (level creation, asset import) — the packaging scripts already live in `Scripts/Deploy/` |
| `Source/EinsteinHatTiler/Private/AssemblyCoded/` | Intrinsics backend for the `Assembly Coded` code path (§4.5) — pending the SIMD sources |

---

## 3. Goals

1. Create infrastructure for a 3D MVP Hat working in Unreal Engine — C++ `KiteShape`, `Hat`, and `MetaHat`.
2. Create infrastructure for Model Development MVP Hat working in JavaScript.
3. Create a commercial channel: Unreal Fab `MvpHat`.
4. Implement a secondary channel: iPhone Marketplace `MvpHat`.
5. Technology mid-point: implemented Config Hat to enable all Spectre versions.
6. Make a minimal model for design use: Hat metalevels.
7. Make infinite-zoom components using an infinite-zoom Material — Penrose thin and thick rhombi.
8. Apply to best-practice pattern for a 3D Unreal Engine Demo Level which uses custom and common materials.
9. Make a commercially available product on Fab.
10. Get a paying customer sale on Fab.

---

## 4. Specifications

### 4.1 Hat

The smallest physical tile unit. A non-periodic monotile that tiles the plane only aperiodically — the geometry must be exact, so vertex offsets are derived from the reference CAD sources in [§5.3](#53-geometry--3d-openscad--stl).

### 4.2 MetaHat

A cluster of Hats (metatiles H, T, P, F) that are themselves substituted recursively. MetaHat introduces the **meta-level** concept: each level up replaces a tile with a cluster of tiles at the next subdivision, which is what produces infinite zoom.

Substitution rules are taken directly from the reference visualizer in [§5.1](#51-the-gold-standard-javascriptweb).

### 4.3 Config surface

Three shape configuration axes drive the whole tiling system:

| Config | Applies to | Effect |
| --- | --- | --- |
| Kite mid-point | `KiteShape` | Raise or lower the mid-point of the kite |
| Hat points | `Hat` | Raise or lower all points of the hat |
| Spectre variants | `Hat` | Enable all Spectre versions — the outliers and the middle |

`FEinsteinHatConfig::CodePath` is a fourth, orthogonal setting. It selects the geometry *backend*
rather than changing the shape — see §4.5.

### 4.4 C++ module (`Source/EinsteinHatTiler`)

#### Geometry

The hat is a **polykite**: eight congruent right kites with angles 60°/90°/120°/90°, laid out on a triangular lattice. Lattice coordinates are integers; they map to the plane as

```
LatticeToPlane(x, y) = (x + y/2, y * sqrt(3)/2)
```

The 13-vertex outline and the 8-kite decomposition live in `EinsteinHatTypes.h` as compile-time tables. The decomposition is **verified exact**: the union of the eight kites has a symmetric difference of zero against the 13-vertex outline, and the areas agree at `8 * sqrt(3)`.

| Table | Contents |
| --- | --- |
| `GetHatOutline()` | 13 lattice vertices, counter-clockwise |
| `GetUnitKite()` | The canonical kite `(0,0), (2,-1), (2,0), (1,1)` |
| `GetHatKites()` | 8 kites, each ordered as 60° apex → 90° → 120° apex → 90°, counter-clockwise |

Because lattice-to-plane is affine, a kite's centre is the midpoint of its long diagonal (the 60° apex and the 120° apex), which is what `KiteCentrePlane` returns.

#### Mesh generation

Each kite is triangulated as a four-triangle fan around that centre point. That single choice is what makes the elevation configs work:

* `FEinsteinHatConfig::KiteMidpointHeight` raises or lowers the fan's centre vertex (Milestone VII).
* `FEinsteinHatConfig::PointHeight` raises or lowers the shared kite corners, i.e. all points of the hat (Milestone VIII).

`FEinsteinHatConfig` also carries `TileScale` (centimetres per lattice unit, default 100 — a hat is about 5 lattice units across) and `Variant`.

#### Classes

| Class | Role |
| --- | --- |
| `AKiteActor` | One kite on a `UProceduralMeshComponent`, centred on its own midpoint. `TileMaterial` overrides the default. `RebuildMesh()` is editor-callable. |
| `AHatShape` | The whole hat as a single procedural mesh. `GetLocalCentre()` returns the local bounding-box centre; `Seed` applies a deterministic yaw (the hook for the seeded patch generator in §5.2). |

Both actors rebuild in `OnConstruction`, so tweaking the config in the details panel updates the mesh live.

#### Blueprint API (`UEinsteinHatLibrary`)

| Function | Returns |
| --- | --- |
| `GetHatOutline(TileScale)` | 13 outline vertices in centimetres |
| `GetHatKiteCentres(TileScale)` | 8 kite centres in centimetres |
| `GetHatKiteCount()` | `8` |
| `BuildHatMesh(Config, …)` | Vertices, triangles, normals, planar UVs for a full hat |
| `BuildKiteMesh(Config, …)` | The same for the single unit kite |

#### Status against the milestones

* **Done** — the Hat outline, the 8-kite decomposition, `KiteActor`, `HatShape`, and the mid-point / point elevation configs (§4.3, Milestone VII–VIII hooks).
* **Reserved** — `EEinsteinHatVariant::Turtle` and `::Spectre`. Selecting either logs a warning and falls back to the Hat; the geometry lands with Milestone IX.
* **Not started** — `MetaHat` substitution (§4.2, Milestone VI), the seeded infinite patch, the Penrose rhombus materials, and the `Assembly Coded` intrinsics backend (§4.5, HP-2 to HP-6).


### 4.5 High-performance C++ — `Assembly Coded`

Geometry generation is on the hot path: every tile in a patch runs the same lattice-to-plane
transform and the same triangle-fan expansion. That makes it a candidate for hand-written SIMD.

`FEinsteinHatConfig::CodePath` selects the implementation:

| `EEinsteinHatCodePath` | Meaning |
| --- | --- |
| `Cpp` | Portable scalar C++ — the reference implementation and the fallback |
| `AssemblyCoded` | Hand-vectorised intrinsics, 256/512-bit SIMD selected at runtime on CPU support |

`AssemblyCoded` is the "Assembly Coded" property from the roadmap. It is a **code-path selector, not
a quality setting**: the output must be identical to `Cpp`, so both backends are validated against
the same outline and kite tables.

#### Where it applies

| Stage | Vectorises | Notes |
| --- | --- | --- |
| Lattice → plane | Yes | 13 outline or 4×8 kite vertices, trivially data-parallel |
| Triangle fan | Partly | Index expansion is integer work; only pays at patch scale |
| `MetaHat` substitution | Yes at scale | Per-tile matrix work — the real win, once §4.2 lands |
| Broad-phase culling / spatial hash | Yes | Patch-scale, not per-tile |
| Procedural mesh upload | No | Engine-side |

Not everything is worth vectorising. Per-tile SIMD over 8 kites is unlikely to beat the scalar path
once call overhead is counted. The honest target is **patch scale** — thousands of tiles — which is
milestone II/VI, not the single-hat MVP. Paths that do not beat scalar should be retired rather than
kept for their own sake (HP-6).

#### Hook point

`UEinsteinHatLibrary::BuildHatMesh` and `BuildKiteMesh` branch on `CodePath`. `AssemblyCoded`
currently logs once and falls back to `Cpp`. The intrinsics backend drops into
`Source/EinsteinHatTiler/Private/AssemblyCoded/` when the sources arrive. The scalar path stays as
the reference — it defines correctness.

---

## 5. Kite Substitutes — Reference Implementations

### 5.1 The "Gold Standard" (JavaScript/Web)
**Project:** [hatviz](https://github.com/isohedral/hatviz) by Craig Kaplan

The official visualizer created by one of the co-discoverers of the Hat tile.

- **Why it's useful:** It contains the precise substitution rules for "Metatiles" (the H, T, P, and F clusters). You can read the `hat.js` file directly to see how the meta-levels are constructed.
- **Tech:** P5.js (JavaScript). It is the perfect reference for the iPhone App's logic.

### 5.2 The Implementation Logic (Python/Math)
**Project:** [Einstein_Tile_Generator](https://github.com/asmoly/Einstein_Tile_Generator) by asmoly (Google Research)

- **What it is:** A robust generator that uses a coordinate system based on the underlying "Kite" grid.
- **Why it's useful:** It includes `seed_to_pattern.py` logic which is vital for the Marketplace MVP. It shows how to generate a unique infinite patch from a single seed value — essential for procedural world-building in Unreal.

### 5.3 Geometry & 3D (OpenSCAD/STL)
**Project:** [aperiodic-monotile](https://github.com/christianp/aperiodic-monotile) by christianp

- **What it is:** A collection of the tile in CAD formats (DXF, STL, OpenSCAD).
- **Why it's useful:** If you want to build the `KiteShape` in Unreal using C++, the `.scad` files provide the exact vertex offsets. It also includes the "Turtle" and "Spectre" variations.

---

## 6. Milestones

| # | Milestone |
| --- | --- |
| I | MVP Hat for JavaScript |
| II | MVP Hat for Unreal Engine |
| III | Unreal Marketplace EinsteinHat |
| IV | iPhone App EinsteinHat |
| V | `KiteShape` |
| VI | `MetaHat` |
| VII | Config on Kite to raise or lower mid-point |
| VIII | Config on Hat to raise or lower all points |
| IX | Config on Hat to enable all Spectre versions — the outliers and the middle |
| X | Meta meta Hat |
| XI | Material: Penrose thin and thick rhombi |
| XII | Penrose thin and thick rhombi working on Kite |
| XIII | Penrose thin and thick rhombi working on Hat |
| XIV | Penrose thin and thick rhombi working on MetaHat |
| XV | Demo Level that uses Penrose materials |
| XVI | Demo Level with common Unreal materials |
| XVII | Payable Unreal Component |
| XVIII | Paying Customer |

### Performance track (`Assembly Coded`)

Cross-cutting rather than sequential — these land alongside the feature milestones above, and only
where the measurement justifies them (§4.5).

| # | Item | Lands with |
| --- | --- | --- |
| HP-1 | `Assembly Coded` code-path property on `FEinsteinHatConfig` | done — selector only |
| HP-2 | CPU feature detection and 256/512-bit dispatch | II |
| HP-3 | Vectorised lattice → plane transform and kite fan | II |
| HP-4 | Vectorised `MetaHat` substitution at patch scale | VI |
| HP-5 | Parity test: `AssemblyCoded` output must match `Cpp` exactly | II |
| HP-6 | Benchmark harness; retire any path that does not beat scalar | II |

---

## 7. Deliveries

Each milestone ships a self-contained package under `Deliveries/`. The source document reuses the
bare name `EinsteinHatTiler/` for all eighteen, which makes them indistinguishable on disk, so every
package is suffixed with its milestone letter — `EinsteinHatTiler.A/` through `EinsteinHatTiler.R/`.

Each package has its own deploy script that copies that delivery's assets out of the project:

| # | Delivery | Package | Deploy |
| --- | --- | --- | --- |
| A | MVP Hat for JavaScript — HTML single-file | `Deliveries/EinsteinHatTiler.A/` | `Scripts/Deploy/deployA.cmd` |
| B | MVP Hat for Unreal Engine | `Deliveries/EinsteinHatTiler.B/` | `Scripts/Deploy/deployB.cmd` |
| C | Unreal Marketplace EinsteinHat | `Deliveries/EinsteinHatTiler.C/` | `Scripts/Deploy/deployC.cmd` |
| D | iPhone App EinsteinHat | `Deliveries/EinsteinHatTiler.D/` | `Scripts/Deploy/deployD.cmd` |
| E | KiteShape — HTML, Unreal, C++ | `Deliveries/EinsteinHatTiler.E/` | `Scripts/Deploy/deployE.cmd` |
| F | MetaHat — HTML, Unreal, C++ | `Deliveries/EinsteinHatTiler.F/` | `Scripts/Deploy/deployF.cmd` |
| G | Config on Kite to raise or lower mid-point | `Deliveries/EinsteinHatTiler.G/` | `Scripts/Deploy/deployG.cmd` |
| H | Config on Hat to raise or lower all points | `Deliveries/EinsteinHatTiler.H/` | `Scripts/Deploy/deployH.cmd` |
| I | Config on Hat to enable all Spectre versions | `Deliveries/EinsteinHatTiler.I/` | `Scripts/Deploy/deployI.cmd` |
| J | Meta meta Hat | `Deliveries/EinsteinHatTiler.J/` | `Scripts/Deploy/deployJ.cmd` |
| K | Material: Penrose thin and thick rhombi | `Deliveries/EinsteinHatTiler.K/` | `Scripts/Deploy/deployK.cmd` |
| L | Penrose rhombi working on Kite | `Deliveries/EinsteinHatTiler.L/` | `Scripts/Deploy/deployL.cmd` |
| M | Penrose rhombi working on Hat | `Deliveries/EinsteinHatTiler.M/` | `Scripts/Deploy/deployM.cmd` |
| N | Penrose rhombi working on MetaHat | `Deliveries/EinsteinHatTiler.N/` | `Scripts/Deploy/deployN.cmd` |
| O | Demo Level that uses Penrose materials | `Deliveries/EinsteinHatTiler.O/` | `Scripts/Deploy/deployO.cmd` |
| P | Demo Level with common Unreal materials | `Deliveries/EinsteinHatTiler.P/` | `Scripts/Deploy/deployP.cmd` |
| Q | Payable Unreal Component | `Deliveries/EinsteinHatTiler.Q/` | `Scripts/Deploy/deployQ.cmd` |
| R | Paying Customer | `Deliveries/EinsteinHatTiler.R/` | `Scripts/Deploy/deployR.cmd` |

The scripts share one manifest table in `Scripts/Deploy/deploy.ps1`, which maps each project path to
its path inside the package. A delivery whose contents are not defined yet (D and G–R) still gets a
script; it reports the manifest as pending rather than inventing files. Any script exits non-zero
when a listed asset is missing, so an incomplete package cannot ship by accident.

`Deliveries/` is build output and is git-ignored.

The layouts for A–F are fixed; G–R gain theirs when those deliveries are specified.

### A. MVP Hat for JavaScript — HTML single-file
```
EinsteinHatTiler.A/
├── Maps/
│   └── ExampleMap.umap
├── Blueprints/
│   ├── BP_EinsteinHatTilerMvp.uasset
│   └── WBP_EinsteinHatTilerMvp.uasset
└── Documentation/
    └── EinsteinHatTilerMvp.html
```

### B. MVP Hat for Unreal Engine
```
EinsteinHatTiler.B/
├── Blueprints/
│   └── BPC_EinsteinHatTilerMin.uasset
└── Maps/
    └── ExampleMap.umap
```
C++ classes:
- `EinsteinHatTypes.h` — a header-only file defining the "DNA" of the shape.
- `KiteActor.h` / `.cpp` — the smallest physical unit.
- `HatShape.h` / `.cpp` — the container actor.

### C. Unreal Marketplace EinsteinHat
```
EinsteinHatTiler.C/
├── Blueprints/
│   └── BPC_EinsteinHatTilerMin.uasset
├── Materials/
│   └── M_HatTileMin.uasset
├── Maps/
│   └── ExampleMap.umap
└── Documentation/
    ├── README.pdf
    └── Icon128.png
```

### D. iPhone App EinsteinHat

Apple project (Xcode). No layout is defined yet, so `deployD.cmd` reports the manifest as pending.

### E. KiteShape — HTML, Unreal, C++
```
EinsteinHatTiler.E/
├── Blueprints/
│   └── BPC_EinsteinHatTilerKite.uasset
├── Materials/
│   └── M_HatTileKite.uasset
├── Maps/
│   └── ExampleMap.umap
└── Documentation/
    └── README.pdf
```

### F. MetaHat — HTML, Unreal, C++
```
EinsteinHatTiler.F/
├── Blueprints/
│   └── BPC_EinsteinHatTiler.uasset
├── Materials/
│   └── M_HatTileMeta.uasset
├── Maps/
│   └── ExampleMap.umap
└── Documentation/
    └── README.pdf
```

### G–R. Remaining milestones

G through R have no defined layout yet — see the package table at the top of this section for their
folder and deploy script. Each gains a concrete layout when the delivery is specified.

---

## 8. Building from an IDE

**Primary IDE: Visual Studio 2026 Insiders** (version 18). The C++ toolchain on this machine is
`C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Tools\MSVC\14.50.35717`.

### Project files

This engine install ships no `GenerateProjectFiles.bat`, so drive UnrealBuildTool directly. Use the
script, which pins the format explicitly:

```
Scripts\GenerateProjectFiles.cmd
```

which runs:

```
dotnet "C:\p\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" ^
    -ProjectFiles -Project="<repo>\UeEinsteinHat\UeEinsteinHat.uproject" ^
    -ProjectFileFormat=VisualStudio2026 -ProjectFileFormat=VisualStudioCode -Game -Engine
```

`VisualStudioCode` additionally writes `.vscode/c_cpp_properties.json` and the
`compileCommands_*.json` database for IntelliSense. Both are machine-specific and git-ignored.

**Visual Studio 18 *is* Visual Studio 2026.** Do not pass `-2022`; it fails with *"Visual Studio C++
2022 installation not found"*. `-ProjectFileFormat=VisualStudio2026` is the correct switch, and it
generates a solution stamped `# Visual Studio Version 18`. Format auto-detection resolves to the
same thing, since VS 2026 is the only Visual Studio installed — so editor-driven generation is
already correct and no `BuildConfiguration.xml` edit is required.

`*.sln`, `*.slnx` and the `.vcxproj` files are git-ignored: they are generated, and they embed
absolute paths. Regenerate after pulling or after changing `Source/`.

### Visual Studio

Open `UeEinsteinHat.sln`. It contains the `UeEinsteinHat` (game) and editor targets; use the
`Development Editor` configuration for day-to-day work.

The generated `.vcxproj` files shell out to `Build.bat` using `$(SolutionDir)`, so they **must** be
built through the solution — building a `.vcxproj` on its own resolves `$(SolutionDir)` to
`Intermediate\ProjectFiles` and fails with *"Unable to find project file"*.

Verified working: MSBuild on the generated project produced `Binaries\Win64\UeEinsteinHat.exe`.

Note for scripting: `msbuild UeEinsteinHat.sln -t:UeEinsteinHat` does **not** work on MSBuild 18 — its
new solution parser forwards the target to every project, including the engine `.csproj` files, and
fails with a wall of `MSB4057`. Build from the IDE, or invoke the `.vcxproj` with
`/p:SolutionDir=<project folder>\` as VS does.

`.vs-support/` is a local Visual Studio scratch folder and is git-ignored.

### Visual Studio Code

`.vscode/tasks.json` defines these tasks:

| Task | Purpose |
| --- | --- |
| `Build Editor (Development)` | Default build (`Ctrl+Shift+B`). Editor must be closed. |
| `Build Game (Development)` | Same compile, safe while the editor is open. |
| `Rebuild Editor (Development)` / `Clean Editor` | Full rebuild and clean. |
| `Generate Project Files` | Runs `Scripts/GenerateProjectFiles.cmd`. |
| `Launch Editor` / `Deploy Package A` | Convenience wrappers. |

Recommended extensions are in `.vscode/extensions.json` (`ms-vscode.cpptools`,
`ms-dotnettools.csharp`). The engine path is hardcoded in `tasks.json` and `launch.json`; change the
`C:/p/UE_5.8` occurrences if the engine moves.

### One caveat that catches everyone

**Live Coding blocks editor target builds.** While the Unreal Editor is running with this project,
any `UeEinsteinHatEditor` build — from Visual Studio, VS Code, or the command line — fails with
*"Unable to build while Live Coding is active"*. Either close the editor, or build the
`UeEinsteinHat` game target instead, which is unaffected.

### Memory

Long compiles on this box can hit the memory ceiling and surface a spurious `cl` internal compiler
error (`C1001`). That is pressure, not a code fault. Re-run with reduced parallelism:

```
Build.bat UeEinsteinHat Win64 Development -Project="<repo>\UeEinsteinHat\UeEinsteinHat.uproject" -MaxParallelActions=1 -NoUBA
```


