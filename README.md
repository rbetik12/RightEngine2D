# RightEngine

Vulkan game engine that is WIP. Currenlty it includes asset system, Vulkan renderer and scene system

## Features (More is coming soon!)

- Flexible Vulkan renderer
- Cross-platform
- Easy to use parallel tasks API

## Main scene with PBR balls

![Main scene](https://i.imgur.com/nyoJtLu.png)

## Requirements
- Python
- Conan >= 2.0
- Vulkan SDK (>= 1.3.236)

## How to build
Currently works only with Windows, but build scripts are easy adoptable for other platforms

1. Launch `prepare_env.bat` to prepare conan profiles and scripts
2. Launch `generate_solution.bat` to install all dependencies and generate solution
3. After all scripts have executed successfully, generated solution can be found at `./build/{platform}` directory
