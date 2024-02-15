# ASoftGlow's The Oregon Trail
## Goals
- [ ] Create a game heavily based on MECC's 1990 The Oregon Trail
- [x] Use only a terminal user interface (TUI)
- [ ] Use minimal dependencies
- [x] Target Windows ≥10 and ubuntu
- [ ] Be enjoyable and replayable
- [x] Save progress
## Build
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
- [SFML](https://github.com/SFML/CSFML) ([dependencies](https://www.sfml-dev.org/tutorials/2.6/compile-with-cmake.php#installing-dependencies))