# RightEngine

Vulkan game engine that is in WIP

## Features

- Flexible Vulkan renderer
- Cross-platform
- Easy to use parallel tasks API

## Requirements
- Python >= 3.10
- Conan >= 2.0
- Vulkan SDK (>= 1.3.268)

## How to build
Currently works only with Windows, but build scripts are easy adoptable for other platforms.

Git can request you login and password from the LFS server, please enter `engine` and `engine` as username and password

1. Launch `prepare_env.bat` to prepare conan profiles and scripts
2. Launch `generate_solution.bat` to install all dependencies and generate solution
3. After all scripts have executed successfully, generated solution can be found at `./build/{platform}` directory
