<p align="center">
  <img height="150" src="logo.svg">
</p>

# libarmcortex

[![.github/workflows/presubmit.yml](https://github.com/libhal/libarmcortex/actions/workflows/presubmit.yml/badge.svg?branch=main)](https://github.com/libhal/libarmcortex/actions/workflows/presubmit.yml)

# [📚 Software APIs](https://libhal.github.io/libarmcortex/api)

Drivers for ARM cortex-m series micro-controllers. Currently supports:

* Cortex-M3
* Cortex-M4
* Cortex-M7

# Installation

## Installing prerequistes

## Install from `libhal-trunk`

`libhal-trunk` is a remote conan server with the latest version of the code.

Run the following to add `libhal-trunk` to your list of conan remote servers.

NOTE: that the `--insert` argument places this server at the highest priority
for conan, meaning updates will be checked at this server first before
attempting to check out servers like the CCI.

```bash
conan remote add libhal-trunk https://libhal.jfrog.io/artifactory/api/conan/trunk-conan --insert
```

Now run this command to install libhal from that server.

```bash
conan install libhal
```
