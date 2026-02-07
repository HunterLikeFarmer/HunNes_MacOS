# HunNes

HunNes is an NES emulator written in C++.

This repository is Linux-first and built with Make.

## Requirements

- C++17 compiler (`g++` or `clang++`)
- GNU Make
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
make -j
```

## Run

Create `rom/` in the repo root and place `.nes` files there.

Then run:

```bash
./build/HunNes
# or
make run
```
