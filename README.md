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

Learn more about Conan profiles in the
[Conan documentation](https://docs.conan.io/2/reference/config_files/profiles.html).

The `conan/profiles` directory contains a set of predefined profiles for
different ARM Cortex-M cores. These profiles are used to specify settings for
conan in order to either compiler the library for a specific target or provide
information to an application about which binaries to download from the conan
package repository. Each profile sets the appropriate compiler flags for the
corresponding core.

The profiles labeled `thumbvX` where `X` is the thumb instruction set version
number. For example, the `thumbv7` profile defines the mininum compiler flags
for the cores capable of using `thumbv7` such as `Cortex-M3` and `Cortex-M4`.
While the `thumbv8` profile sets the compiler flags for the ARM `Cortex-M23` and
`Cortex-M33` cores. Here is an example profile for `thumbv7`:

```jinja
[settings]
compiler=gcc
compiler.cppstd=20
compiler.libcxx=libstdc++
compiler.version=12.2
os=baremetal
arch=thumbv7
arch.float_abi={{ float_abi }}
arch.fpu={{ fpu }}
arch.processor={{ cpu }}

[tool_requires]
arm-gnu-toolchain/12.2.1

[conf]
tools.build:cflags=["-mfloat-abi={{ float_abi }}", "-mcpu={{ cpu }}", "-mfpu={{ fpu }}", "-fno-exceptions", "-fno-rtti"]
tools.build:cxxflags=["-mfloat-abi={{ float_abi }}", "-mcpu={{ cpu }}", "-mfpu={{ fpu }}", "-fno-exceptions", "-fno-rtti"]
tools.build:exelinkflags=["--specs=nano.specs", "--specs=nosys.specs", "-mfloat-abi={{ float_abi }}", "-mcpu={{ cpu }}", "-mfpu={{ fpu }}", "-fno-exceptions", "-fno-rtti"]
```

Conan uses jinja2 templates to fill in the areas marked with
`{{ <variable name> }}`. The complete profiles such as `cortex-m0`, `cortex-m3`,
and `cortex-m4f` set the appropriate flags and import their respective `thumbvX`
profile. For example lets take a look at `cortext-m0` and `cortex-m4f`.

```jinja
{% set cpu = "cortex-m0" %}
{% include "thumbv6" %}
```

```jinja
{% set cpu = "cortex-m4" %}
{% set float_abi = "hard" %}
{% set fpu = "auto" %}
{% include "thumbv7" %}
```

When `cortex-m4f` is selected as the conan profile, the contents of the profile
are expanded and all templates are replaced, allowing a library or application
to be compiled for that processor.

Also notice how `cortex-m0` does not have a field for `float_abi`? Thats because
`thumbv6` do not support floating point units and their instructions, so the
floating point ABI compiler flags can be set to `soft` by default.

## Supporting new Processors

To add additional `cortex-mX` processors, simply make a new file in this repo's
`conan/profile` directory with the name of the processor following the format
`cortex-mX` where `X` is the processor number. Modifiers can be added to the end
to specify additional details like `f` for single precisions floating point
hardware support, `d` for double precision floating point hardware support, and
`neon` for devices that support the `neon` floatign point unit. Follow the
same template as the other profiles and make sure that every `jinja2` template
is accounted for in the variable set before using the `{% include "thumbvY" %}`
line.

What if the `thumbv` profile you need doesn't exist? Simply take the others as
reference and add whichever fields are necessary to support that instruction
set.

## Creating platform profiles

Platform profiles take a processor profile and combine it with platform specific
details. These details must include setting the global `platform` option and
any other profile related details that can be used for a build.

Lets consider two examples, the `lpc4078` profile.

`lpc4078` has a profile fragment called `lpc40` which specifies generic info.

```jinja2
[options]
*:platform={{ platform }}

[buildenv]
LIBHAL_PLATFORM={{ platform }}
LIBHAL_PLATFORM_LIBRARY=lpc40
```

`lpc4078`'s profile looks like this:

```jinja2
{% include "cortex-m4f" %}
{% set platform = "lpc4078" %}
{% include "lpc40" %}
```

The `lpc4078` microcontroller uses a `cortex-m4f` processor, so that is
included in this profile. Next we set the `platform` variable. All libhal
platform profiles MUST have a platform name specified in their `[options]`
section of their profile. And finally the profile includes the `lpc40` which
provides the final portion of the profile.

> Note that the conan profiles always override duplication information with the
last occurrences of the duplicate.

A profile should be made for every supported microcontroller in a platform
library's chip family.

By creating and using these profiles, you can easily compile the library for
different platforms and MCUs while ensuring the correct compiler flags are used.

## How linker scripts work

TBD

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details.

## License

Apache 2.0; see [`LICENSE`](LICENSE) for details.

## Disclaimer

This project is not an official Google project. It is not supported by
Google and Google specifically disclaims all warranties as to its quality,
merchantability, or fitness for a particular purpose.
