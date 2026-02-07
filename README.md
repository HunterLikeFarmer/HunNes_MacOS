# HunNes

HunNes is an NES emulator written in C++.

This repository is Linux-first and built with CMake.

## Requirements

- C++17 compiler (`g++` or `clang++`)
- CMake 3.16+
- SDL2 development package

## Install SDL2

Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev
```

Fedora:

```bash
sudo dnf install SDL2-devel
```

Arch:

```bash
sudo pacman -S sdl2
```

## Build

From repo root:

```bash
cmake -S . -B build
cmake --build build -j
```

## Run

Create `rom/` in the repo root and place `.nes` files there.

Then run:

```bash
./build/HunNes
```
