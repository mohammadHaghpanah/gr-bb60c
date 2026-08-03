# BB60C Sweep example

Polished Qt GUI and GRC flowgraph for the `bb60c.bb60c_sweep` block.

## Run

From the repository root:

```bash
./run_bb60c_sweep_gui.sh
```

Or:

```bash
./examples/bb60c_sweep/run_untitled.sh
```

## Files

| File | Role |
|------|------|
| `bb60c_sweep_gui.py` | Application UI (edit this) |
| `bb60c_sweep.qss` | Stylesheet |
| `untitled.grc` | Reference flowgraph |
| `untitled.py` | GRC-generated; may be overwritten |
| `run_untitled.sh` | Launches the polished GUI via repo root script |

## Notes

- Do not rely on GRC **Generate** to produce the polished UI; it regenerates `untitled.py`.
- Set GRC **Run Command** to `./run_bb60c_sweep_gui.sh` (repo root) when executing from Companion.
- Time Sink: Trigger Mode = Tag, Tag Key = `sweep_start`.
