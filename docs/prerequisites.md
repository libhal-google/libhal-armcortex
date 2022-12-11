# How to install prerequisites for libarmcortex

NOTE: These install steps are not the only possible way to install these
binaries, just that these are very easy ways to install these prereqs.

- ARM cross compiler: `arm-none-eabi-g++` version 11.0 or above

## Installing ARM cross compiler

### Mac

```
brew install --cask gcc-arm-embedded
```

### Linux (untested)

Use xPack to make installation consistent for your platform, requires Node.js
and NPM:

```
npm install xpm
xpm install --global @xpack-dev-tools/arm-none-eabi-gcc@latest --verbose
echo "export PATH=\"$PATH:~/.local/xPacks/@xpack-dev-tools/arm-none-eabi-gcc/11.3.1-1.1.2/.content/bin/\"" >> ~/.bashrc
source ~/.bashrc
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

## Using libhal-trunk (RECOMMENDED)

The "trunk" repository represents the latest packaged code based on github.

This command will insert `libhal-trunk` as the first server to check before
checking the conan center index. The second command will enable revision mode
which is required to use `libhal-trunk` in projects.

```bash
conan remote add libhal-trunk https://libhal.jfrog.io/artifactory/api/conan/trunk-conan --insert
conan config set general.revisions_enabled=True
```