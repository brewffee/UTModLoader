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

## Building
You will need (this is subject to change):
- [cmake](https://cmake.org/): version  >= 3.29
- [libzip](https://www.libzip.org/): version  >= 1.11.2
- [libunarr](https://github.com/selmf/unarr): version  >= 1.1.1

### Dependencies (Linux)

#### Arch Linux
Replace `yay` with your AUR helper of choice
```
yay -S --needed git base-devel g++ cmake libzip libunarr
```

### Dependencies (Windows)

Requires [MSYS2](https://www.msys2.org/) and [mingw-w64](https://mingw-w64.org/) to be installed at `C:\msys64\`
```
pacman -S --needed git base-devel mingw-w64-x86_64-{gcc,headers-git,cmake,libzip}
```


```
git clone https://aur.archlinux.org/libunarr.git
cd libunarr
makepkg -si
```

### Building
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -B ./cmake-build-debug
cmake --build ./cmake-build-debug --target UTModLoader -j $(nproc)
```
