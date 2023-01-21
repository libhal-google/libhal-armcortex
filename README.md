<p align="center">
  <img height="150" src="logo.svg">
</p>

# libhal-armcortex

[![✅ Checks](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml/badge.svg)](https://github.com/libhal/libhal-armcortex/actions/workflows/ci.yml)
[![Coverage](https://libhal.github.io/libhal-armcortex/coverage/coverage.svg)](https://libhal.github.io/libhal-armcortex/coverage/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b084e6d5962d49a9afcb275d62cd6586)](https://www.codacy.com/gh/libhal/libhal-armcortex/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=libhal/libhal-armcortex&amp;utm_campaign=Badge_Grade)
[![GitHub stars](https://img.shields.io/github/stars/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/network)
[![GitHub issues](https://img.shields.io/github/issues/libhal/libhal-armcortex.svg)](https://github.com/libhal/libhal-armcortex/issues)
[![Latest Version](https://libhal.github.io/libhal-armcortex/latest_version.svg)](https://github.com/libhal/libhal-armcortex/blob/main/conanfile.py)
[![ConanCenter Version](https://repology.org/badge/version-for-repo/conancenter/libhal-armcortex.svg)](https://conan.io/center/libhal-armcortex)

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
