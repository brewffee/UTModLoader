# UTModLoader
UTModLoader is a tool for extracting and installing mods for the game [Unreal Tournament](https://www.oldunreal.com/).

## Usage
```
utmodloader [options] <command> <path>
```

### Options
- `-h, --help` - Displays the help message
- `-v, --version` - Displays version information

### Commands
- `extract` - Searches <path> for mod files and extracts them to the application directory (`~/.local/share/UTModLoader/store` on Linux and `%APPDATA%\UTModLoader\store` on Windows)

Valid mod file types include `.zip`, `.rar`, and `.umod`.

## Building from source
You will need (this is subject to change):
- [cmake](https://cmake.org/): version  >= 3.29
- [libzip](https://www.libzip.org/): version  >= 1.11.2
- [libunrar](https://www.rarlab.com/rar_add.htm): version  >= 7.1.3

### Dependencies (Linux)

#### Arch Linux
```
pacman -S --needed git base-devel g++ cmake libzip libunrar
```

### Dependencies (Windows)

Requires [MSYS2](https://www.msys2.org/) to be installed at `C:\msys64\`
```
pacman -S --needed git base-devel unrar mingw-w64-x86_64-{gcc,headers-git,cmake,libzip}
```

Get UnRAR.dll from [RARLAB](https://www.rarlab.com/rar_add.htm) and install it to the default location (`C:\Program Files (x86)\UnrarDLL`)

### Building
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -B ./cmake-build-debug
cmake --build ./cmake-build-debug --target UTModLoader -j $(nproc)
```
