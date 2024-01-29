# Build
### Windows
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
### Linux
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Dependencies
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended)
- [SFML](https://github.com/SFML/CSFML)