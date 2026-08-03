# gr-bb60c

GNU Radio Out-Of-Tree (OOT) module for the **Signal Hound BB60C** spectrum analyzer.
Provides a continuous sweep source block that streams spectrum bins as float samples
with `sweep_start` tags for stationary QT GUI Time Sink displays.

## Features

- `bb60c.bb60c_sweep` — continuous sweep source (float stream)
- Runtime parameter setters (start/stop freq, RBW/VBW, detector, etc.)
- Message ports: `num_points`, `meta`
- Optional second stream: `int32` sweep length per sample
- Example Qt GUI with side-panel controls and QSS theme

## Requirements

- GNU Radio 3.10+
- Signal Hound BB Series API (`bb_api.h`, `libbb_api`) — typically under `/usr/local`
- CMake ≥ 3.8
- C++17-capable compiler
- Python 3 + PyQt5 (for the example GUI)
- Connected BB60C hardware (for live capture)

Install the [Signal Hound SDK](https://signalhound.com/software/signal-hound-sdk/) so CMake can find:

- Header: `bb_api.h` (e.g. `/usr/local/include`)
- Library: `libbb_api.so` (e.g. `/usr/local/lib`)

## Build & install

```bash
git clone https://github.com/haghpanah/gr-bb60c.git
cd gr-bb60c
mkdir build && cd build
cmake ..
cmake --build . -j"$(nproc)"
sudo cmake --install .
sudo ldconfig
```

After install, restart GNU Radio Companion and **Reload Block Library**.

### Optional: install from a local build without full system install

```bash
./INSTALL_FIXED_LIB.sh
```

This copies the built shared library into `lib_runtime/` for local runs via the helper scripts.

## Quick start (example GUI)

With the module installed (or `lib_runtime` set up for local use):

```bash
./run_bb60c_sweep_gui.sh
```

Or from the example folder:

```bash
./examples/bb60c_sweep/run_untitled.sh
```

### GRC flowgraph

Open `examples/bb60c_sweep/untitled.grc`. The **Run Command** should point to
`./run_bb60c_sweep_gui.sh` (from the repo root) so GRC launches the polished GUI
instead of regenerating/overwriting `untitled.py`.

| File | Role |
|------|------|
| `examples/bb60c_sweep/bb60c_sweep_gui.py` | Polished application — edit this for UI |
| `examples/bb60c_sweep/bb60c_sweep.qss` | Stylesheet |
| `examples/bb60c_sweep/untitled.grc` | Flowgraph reference |
| `examples/bb60c_sweep/untitled.py` | GRC-generated; disposable |

### Time Sink tips

- **Trigger Mode** = Tag  
- **Trigger Tag Key** = `sweep_start`  
- Wire `num_points` → Message Pair to Var → `time_sink.set_nsamps`

## Module layout

```
gr-bb60c/
├── include/gnuradio/bb60c/   # Public API headers
├── lib/                     # C++ block implementation
├── python/bb60c/            # Python bindings
├── grc/                     # GRC block YAML
├── examples/bb60c_sweep/    # Example GUI + flowgraph
├── docs/                    # Doxygen
└── apps/                    # Optional apps
```

## License

GPL-3.0-or-later — see [LICENSE](LICENSE) and SPDX headers in source files.

## Author

Mohammad Haghpanah
