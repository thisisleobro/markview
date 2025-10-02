### Build
```batch
::from build folder call
cmake -S .. -B . -G "MinGW Makefiles" -D CMAKE_EXPORT_COMPILE_COMMANDS=ON

:: for release
cmake -S .. -B . -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release

:: and then
make && markview.exe ..\README.md
```