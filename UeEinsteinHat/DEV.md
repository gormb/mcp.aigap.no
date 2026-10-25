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
| `.vscode/mcp.json` | Points the editor MCP tooling at `http://127.0.0.1:8000/mcp` |
| `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` | Engine-generated directories (git-ignored, recreated on build) |

### Getting Started
1. Open `UeEinsteinHat.uproject` with Unreal Engine 5.8 (generates missing intermediate binaries/cache).
2. The editor exposes MCP tools at `http://127.0.0.1:8000/mcp`, configured via `.vscode/mcp.json`.
3. There is no `Source/` module yet — the project is Blueprint-only until the `KiteShape` / `Hat` / `MetaHat` C++ classes land (Milestone V–VI).

### Planned additions
| Path | Purpose |
| --- | --- |
| `Source/EinsteinHatTiler/` | Runtime C++ module: `EinsteinHatTypes.h`, `KiteActor`, `HatShape` |
| `Content/Blueprints/` | `BPC_EinsteinHatTiler*` Blueprint containers |
| `Content/Materials/` | `M_HatTile*` tile and Penrose rhombus materials |
| `Scripts/` | Editor automation (level creation, asset import) |

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

Three configuration axes drive the whole tiling system:

| Config | Applies to | Effect |
| --- | --- | --- |
| Kite mid-point | `KiteShape` | Raise or lower the mid-point of the kite |
| Hat points | `Hat` | Raise or lower all points of the hat |
| Spectre variants | `Hat` | Enable all Spectre versions — the outliers and the middle |

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

---

## 7. Deliveries

Each milestone ships a self-contained folder. Every layout below nests under `EinsteinHatTiler/`.

### A. MVP Hat for JavaScript — HTML single-file
```
EinsteinHatTiler/
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
EinsteinHatTiler/
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
EinsteinHatTiler/
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
Apple project (Xcode).

### E. KiteShape — HTML, Unreal, C++
```
EinsteinHatTiler/
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
EinsteinHatTiler/
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
| # | Delivery |
| --- | --- |
| G | Config on Kite to raise or lower mid-point |
| H | Config on Hat to raise or lower all points |
| I | Config on Hat to enable all Spectre versions, the outliers and the middle |
| J | Meta meta Hat |
| K | Material: Penrose thin and thick rhombi |
| L | Penrose thin and thick rhombi working on Kite |
| M | Penrose thin and thick rhombi working on Hat |
| N | Penrose thin and thick rhombi working on MetaHat |
| O | Demo Level that uses Penrose materials |
| P | Demo Level with common Unreal materials |
| Q | Payable Unreal Component |
| R | Paying Customer |
