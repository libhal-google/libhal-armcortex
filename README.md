# libhal-armcortex

[![✅ Checks](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml/badge.svg)](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml)
[![Coverage](https://libhal.github.io/libhal-armcortex/coverage/coverage.svg)](https://libhal.github.io/libhal-armcortex/coverage/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b084e6d5962d49a9afcb275d62cd6586)](https://www.codacy.com/gh/libhal/libhal-armcortex/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=libhal/libhal-armcortex&amp;utm_campaign=Badge_Grade)
[![GitHub stars](https://img.shields.io/github/stars/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/network)
[![GitHub issues](https://img.shields.io/github/issues/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/issues)
[![Latest Version](https://libhal.github.io/libhal-armcortex/latest_version.svg)](https://github.com/libhal/libhal-armcortex/blob/main/conanfile.py)

libhal-armcortex is a processor library in the libhal ecosystem. It provides a
set of drivers and functions for ARM Cortex processors, allowing developers to
write portable and efficient embedded software.

## 📚 Software APIs & Usage

To learn about the available drivers and APIs see the
[Doxygen](https://libhal.github.io/libhal-lpc40/api)
documentation page or look at the
[`include/libhal-lpc40`](https://github.com/libhal/libhal-lpc40/tree/main/include/libhal-lpc40)
directory.

To see how each driver is used see the
[`demos/`](https://github.com/libhal/libhal-lpc40/tree/main/demos) directory.

## 🧰 Setup

Following the
[🚀 Getting Started](https://libhal.github.io/2.1/getting_started/)
instructions.

## 📡 Installing Profiles

Profiles define which platform you mean to build your project against. These
profiles are needed for code and demos in this repo and for applications that
wish to execute on an lpc40 device.

```bash
conan config install -sf conan/profiles/ -tf profiles https://github.com/libhal/libhal-armcortex.git
```

Note that running these functions is safe. THey simply overwrite the old files
with the latest files. So running this for `libhal-armcortex` between this and
other platform libraries is fine.

## 🏗️ Building Demos

To build demos, start at the root of the repo and execute the following command:

```bash
conan build demos -pr cortex-m4f -s build_type=Debug
```

This will build the demos for the `cortex-m4f` microcontroller in `Debug` mode.
Replace `cortex-m4f` with any of the other profiles. Available profiles are:

- `cortex-m0`
- `cortex-m0plus`
- `cortex-m1`
- `cortex-m3`
- `cortex-m4`
- `cortex-m4f`

## 🏁 Startup & Initialization

Startup function to initialize the data section. This is REQUIRED for systems
that do not load themselves from storage into RAM. If the executable is executed
from flash then `hal::cortex_m::initialize_data_section()` is required.

```C++
#include <libhal-armcortex/startup.hpp>

hal::cortex_m::initialize_data_section();
```

The `arm-gnu-toolchain` package provides the `crt0.s` startup file which
initializes the BSS (uninitialized) section of memory. If this startup file
is not used, then a call to `hal::cortex_m::initialize_bss_section()` is
required.

```C++
#include <libhal-armcortex/startup.hpp>

hal::cortex_m::initialize_data_section();
hal::cortex_m::initialize_bss_section();
```

If the device has an FPU (floating point unit) then a call to
`hal::cortex_m::initialize_floating_point_unit()` is required before any
floating point unit registers or floating point instructions.

```C++
#include <libhal-armcortex/system_control.hpp>

hal::cortex_m::initialize_floating_point_unit();
```

## Using `conan/profiles`

This processor library contains profiles templates:

- `thumbv6`
- `thumbv7`
- `thumbv8`

And complete profiles:

- `cortex-m0`
- `cortex-m0plus`
- `cortex-m1`
- `cortex-m3`
- `cortex-m4`
- `cortex-m4f`

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details.

## License

Apache 2.0; see [`LICENSE`](LICENSE) for details.

## Disclaimer

This project is not an official Google project. It is not supported by
Google and Google specifically disclaims all warranties as to its quality,
merchantability, or fitness for a particular purpose.
