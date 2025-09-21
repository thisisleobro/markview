### Build
From build folder
```bat
:: Release
cmake -S .. -B . -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release
:: Debug
cmake -S .. -B . -G "MinGW Makefiles" -D CMAKE_EXPORT_COMPILE_COMMANDS=ON

make && simple-markdown.exe
```