# ASoftGlow's The Oregon Trail
A TUI game heavily based on MECC's 1990 The Oregon Trail

## Goals
- [x] Use only a terminal user interface (TUI)
- [ ] Use minimal dependencies
- [x] Target Windows ≥10 and ubuntu
- [ ] Be enjoyable and replayable
- [x] Save progress
- [ ] Original soundtrack

## Build
### CMake Options
|Option|Description|
|------|-|
|TTY   |Build without window specific features like file dialogs|
|MUTE  |Build without sound|

### Windows
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
### Linux
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Static Libraries
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended) ([dependencies](https://github.com/btzy/nativefiledialog-extended?tab=readme-ov-file#dependencies))
- [PortAudio](https://github.com/PortAudio/portaudio)
- [libsnfile](https://github.com/libsndfile/libsndfile)