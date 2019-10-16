# Nano ImGui UI

This project is a standalone gui for [nano-node](https://github.com/nanocurrency/nano-nano)

## Installation

CMake is used for building. C++14 is required. Other dependencies are managed with git submodules:

```
git clone --recursive https://github.com/argakiig/nano_imgui.git
mkdir build; pushd build; cmake ..; cmake --build . --target nano_imgui --config Release; popd
```

On Windows, some flags may need to be passed to cmake:

`-A "x64"` to pick the correct Boost libraries

Adding `-- -m:<threads>`(Windows) `-- -j<threads>`(others) to the end of `cmake --build ...` can speed up build times