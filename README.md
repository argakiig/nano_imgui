# Nano ImGui UI

This project is a standalone gui for [nano-node](https://github.com/nanocurrency/nano-nano)

## Installation

CMake is used for building. C++14 is required. Other dependencies are managed with git submodules:

```
git clone --recursive https://github.com/argakiig/nano_imgui.git
cmake .
make
```

On Windows, some flags may need to be passed to cmake:

`-A "x64"` to pick the correct Boost libraries

