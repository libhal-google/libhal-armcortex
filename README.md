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

# Using Arm GNU Toolchain

```
[tool_requires]
gnu-arm-embedded-toolchain/11.3.0
```

# Usage

## Initializing RAM and co-processors

Before an embedded application can properly start, it needs to initialize its
RAM and any other co-processors it may need at runtime.
