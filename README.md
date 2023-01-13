<p align="center">
  <img height="150" src="logo.svg">
</p>

# libhal-armcortex

[![✅ Checks](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml/badge.svg)](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml)
[![Code Coverage](https://libhal.github.io/libhal-armcortex/coverage/coverage.svg)](https://libhal.github.io/libhal-armcortex/coverage/)

# [📚 Software APIs](https://libhal.github.io/libhal-armcortex/api)

Drivers for ARM cortex-m series micro-controllers. Currently supports:

* Cortex-M4
* Cortex-M4F

> Plan to support all of the other Cortex M series chips very soon!

# Setup

## [Installing libhal prereqs](https://libhal.github.io/setup/)

## Installing Arm GNU Toolchain

> NOTE: These install steps are not the only possible way to install these
> binaries, just that these are very easy ways to install these prereqs.

**Required ARM cross compiler is `arm-none-eabi-g++` version 11.0 or above**

### Mac OSX

```zsh
brew install --cask gcc-arm-embedded
```

### Linux

> The version (10.2.1) used here is too old for libhal which requires 11.3 and
> above. So you must use the steps below
>
> ```
> sudo apt install gcc-arm-none-eabi
> ```

Link to the linux binaries: [Arm GNU Toolchain
Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).

You will also need to add this to your shell's `PATH` variable as well. Or use
the argument `-DTOOLCHAIN_PATH="path/to/your/gnu-arm-toolchain"

### Windows

> The version (10.2.1) used here is too old for libhal which requires 11.3 and
> above. So you must use the steps below
>
>```
>choco install gcc-arm-embedded
>```

Click this link to download the Windows toolchain installer.

[arm-gnu-toolchain-11.3.rel1-mingw-w64-i686-arm-none-eabi.exe](https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-mingw-w64-i686-arm-none-eabi.exe?rev=674f6ef06614499dad033db88c3452b3&hash=B2AAC9DBE66448116B07ED6C0BB7B71EAD875426)

NOTE: At the end of the installation program make sure to check the box for
**"Add to PATH environment variable"**. If this is not done, the compiler will
not be executable from the command line. If this happens, re-run the installer
and check the box.

This executable along with others others can be found here:
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads.

### Verifying Installation

To check the version run the command `arm-none-eabi-g++ --version`. It should
look like this:

```
$ arm-none-eabi-g++ --version
arm-none-eabi-g++ (Arm GNU Toolchain 11.3.Rel1) 11.3.1 20220712
Copyright (C) 2021 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

# Usage

## Initializing RAM and co-processors

Before an embedded application can properly start, it needs to initialize its
RAM and any other co-processors it may need at runtime.
