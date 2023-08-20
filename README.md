# libhal-armcortex

[![✅ Checks](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml/badge.svg)](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml)
[![Coverage](https://libhal.github.io/libhal-armcortex/coverage/coverage.svg)](https://libhal.github.io/libhal-armcortex/coverage/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b084e6d5962d49a9afcb275d62cd6586)](https://www.codacy.com/gh/libhal/libhal-armcortex/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=libhal/libhal-armcortex&amp;utm_campaign=Badge_Grade)
[![GitHub stars](https://img.shields.io/github/stars/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/network)
[![GitHub issues](https://img.shields.io/github/issues/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/issues)

libhal-armcortex is a processor library in the libhal ecosystem. It provides a
set of drivers and functions for ARM Cortex processors, allowing developers to
write portable and efficient embedded software.

## 📚 Software APIs & Usage

To learn about the available drivers and APIs see the
[libhal cortex_m APIs](https://libhal.github.io/2.2/api/namespacehal_1_1cortex__m/)
documentation page or look at the
[`include/libhal-armcortex`](https://github.com/libhal/libhal-armcortex/tree/main/include/libhal-armcortex)
directory.

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
compiler.version=12.3
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

## How Linker Scripts Work

Linker scripts are used to control the memory layout of the final binary file.
They define the memory regions and sections of the binary, and specify where and
how the linker should place the code and data.

In libhal-armcortex, there is a standard linker script named `standard.ld`
located in the `linker_scripts/libhal-armcortex` directory. This script is
included by other linker scripts in the library to provide a common base
configuration. When `lihal-armcortex` is added as a dependancy of a library or
application the path to the linker scripts in `linker_scripts` directory are
added to the linker flags of the build, making them accessible within other
linker scripts via the `INCLUDE "libhal-armcortex/standard.ld` command.

Here is the content of `standard.ld`:

```ld
INCLUDE "libhal-armcortex/third_party/standard.ld"
```

This script includes another script `third_party/standard.ld` which contains the
actual linker commands. This script has 4 variables that must be defined for
the linker script to work as intended. These variables are

- **`__flash`**: Memory mapped flash memory start address
- **`__flash_size`**: Size of flash memory
- **`__ram`**: Start of RAM address
- **`__ram_size`**: RAM size
- **`__stack_size`** (optional): Size of stack memory. Why is this important?
  Stack memory is needed for functions to operate. It provides them with the
  memory hold local variables. This value provides a safety buffer for the
  system's stack memory. If an application uses up enough statically defined
  memory as to leave no room for the applications stack, exceeds this amount,
  then the linker script will issue an error about running out of memory.

Currently, libhal only provides `standard.ld` which supports a single memory
mapped flash and ram block.

Additional linker scripts for multi-ram, multi-flash, and execute from RAM only
systems are planned to be provided at a later date when systems with those
requirements appear in the ecosystem.

## Using Linker Scripts in a Platform Library

Linker scripts are used in platform libraries to define the memory layout
specific to the platform. For example, in the libhal-lpc40 library, there are
several linker scripts in the `linker_scripts/libhal-lpc40` directory. Each of
these scripts corresponds to a specific model of the lpc40 series of MCUs.

Here is an example of the `lpc4076.ld` linker script from libhal-lpc40:

```ld
__flash = 0x00000000;
__flash_size = 256K;
__ram = 0x10000000;
__ram_size = 64K;
__stack_size = 1K;

INCLUDE "libhal-armcortex/standard.ld"
```

This script defines the memory layout for the lpc4076 MCU. It specifies the
start addresses and sizes of the flash and RAM memory regions, as well as the
stack size. It then includes the standard linker script from libhal-armcortex to
provide the common base configuration.

To create a new platform profile, you would create a new linker script similar
to this one, but with the memory layout specific to your platform. You would
then use this linker script when building your platform library.

Every supported microcontroller in the platform library should have an
associated linker script with its name like so `platform_name.ld`. The platform
library should have a `package_info()` section like this in their
`conanfile.py`:

```python
def package_info(self):
    self.cpp_info.set_property("cmake_target_name", "libhal::lpc40")
    self.cpp_info.libs = ["libhal-lpc40"]

    if self._bare_metal and self._is_me:
        linker_path = os.path.join(self.package_folder, "linker_scripts")
        link_script = "-Tlibhal-lpc40/" + str(self.options.platform) + ".ld"
        self.cpp_info.exelinkflags = ["-L" + linker_path, link_script]
```

This will add the platform linker scripts to the linker script flags. The
`self._bare_metal` property determines if the OS is equal to `baremetal`.
The `self._is_me` property determines if the platform option is one of the
microcontrollers supported by this library. This together check if the system
is being cross built and targeting a platform we have a linker script for. In
other cases, if this isn't a bare metal build, for example a build for unit
testing, then we want to ensure that the linker scripts are not added to the
linker flags. Will cause an error because the linkers for applications on a
OS like linux or mac will not match the one in the bare metal case.

## Customizing Profiles & Linker Scripts

In libhal, customization of profiles and linker scripts is a straightforward
process. This allows developers to tailor the build process to their specific
needs and hardware configurations.

To customize a profile, you need to create a new profile. You have the option to
simple include one of the platform's pre-made profile files. Once you finished
making your profile, you then, set the platform option name to anything other
than the available platforms in the library. It's important to note that Conan
profiles always prioritize and overwrite the fields based on the last one
defined. This means that any settings in your custom profile will overwrite the
corresponding settings in the included platform profile.

For example, the libhal-lpc40 library has several predefined platforms:
`lpc4072`, `lpc4074`, `lpc4076`, `lpc4078`, and `lpc4088`. If you want to
customize the build process for this library, you could create a new profile
named `lpc40xx`. This profile would include the `lpc40` platform profile and set
the platform option to `lpc40xx`.

```jinja2
include(lpc4078)

[options]
platform=lpc40xx
```

Setting the platform option to `lpc40xx` causes the recipe to skip adding a
linker to the linker flags. This allows you, as the developer, to choose your
own linker script for your build. This is required when the platform option is
set to a value not recognized by the library.

By customizing profiles and linker scripts in this way, you can easily adapt the
build process to different hardware configurations and project requirements.

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details.

## License

Apache 2.0; see [`LICENSE`](LICENSE) for details.

## Disclaimer

This project is not an official Google project. It is not supported by
Google and Google specifically disclaims all warranties as to its quality,
merchantability, or fitness for a particular purpose.
