title: The BB60C OOT Module
brief: GNU Radio OOT module for Signal Hound BB60C continuous spectrum sweep
tags:
  - sdr
  - spectrum
  - bb60c
  - signalhound
author:
  - Mohammad Haghpanah
copyright_owner:
  - Mohammad Haghpanah
license: GPL-3.0-or-later
gr_supported_version: 3.10
---
Continuous BB60C spectrum sweep source for GNU Radio. Streams sweep bins as
float samples with `sweep_start` tags for QT GUI Time Sink, plus runtime
setters and `num_points` / `meta` message ports.
