# How to install prerequisites for libarmcortex

NOTE: that these prerequisites aren't the only way one can use and install
the set of tools for libarmcortex, but these are the recommended way of doing
it.

## Prerequisites

- `arm-none-eabi-g++`: 11.0 or above (ARM cross compiler)

## Installing `arm-none-eabi-g++` Recommended way

libarmcortex is meant to be compiled on a host machine for unit test or
to be compiled using the GCC C++ bare metal ARM compiler to build code.

To install the bare metal ARM compile follow these steps:

### Ubuntu/Debian

```bash
sudo apt install arm-none-eabi-gcc
```

### Mac

```
brew install --cask gcc-arm-embedded
```

### Windows

Click this link to download the Windows toolchain installer.

[arm-gnu-toolchain-11.3.rel1-mingw-w64-i686-arm-none-eabi.exe](https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-mingw-w64-i686-arm-none-eabi.exe?rev=674f6ef06614499dad033db88c3452b3&hash=B2AAC9DBE66448116B07ED6C0BB7B71EAD875426)

NOTE: At the end of the installation program make sure to check the box for
"Add to PATH environment variable". If this is not done, the compiler will not
be executable from the command line. If this happens, re-run the installer and
check the box.

This executable along with others others can be found here:
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads.

### Installing by other means `arm-none-eabi-g++`

You can download the prebuilt binaries for your platform and manually add
them to your systems PATH variable, if you do not, or cannot install it
globally by downloading the `.tar.gz` or `.zip` files from:

https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

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
