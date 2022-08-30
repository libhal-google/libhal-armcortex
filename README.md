# libarmcortex

[![.github/workflows/presubmit.yml](https://github.com/libhal/libarmcortex/actions/workflows/presubmit.yml/badge.svg?branch=main)](https://github.com/libhal/libarmcortex/actions/workflows/presubmit.yml)

Drivers for ARM cortex-m series microcontrollers. Currently supports:

* M3
* M4
* M7

### Installing ARM toolchain

libhal is platform and architecture agnostic but if you do plan to build
application for ARM Cortex M class microcontrollers then you will want to follow
these steps:

- Ubuntu/Debian: `sudo apt install arm-none-eabi-gcc`
- Mac: `brew install --cask gcc-arm-embedded`
- Windows: Installer found here: [GNU Arm Embedded Toolchain Downloads](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- Or you can download the prebuilt binaries for your platform and manually
  add them to your systems PATH variable, if you do not, or cannot install
  it globally.