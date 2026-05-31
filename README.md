# HunNes

HunNes is an NES emulator written in C++.

This repository is configured for macOS and built with Make.

## Requirements

- macOS
- Xcode Command Line Tools
- [Homebrew](https://brew.sh/)
- SDL2

## Install Dependencies

Install the Xcode Command Line Tools if they are not already available:

```bash
xcode-select --install
```

Install SDL2 with Homebrew:

```bash
brew install sdl2
```

## Build

From repo root:

```bash
make -j"$(sysctl -n hw.logicalcpu)"
```

## Run

Create `rom/` in the repo root and place `.nes` files there.

Then run:

```bash
./run.sh
# or
make run
```
