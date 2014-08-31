## UTLauncher

UTLauncher is a standalone application to join UT4 servers. You will need latest raxxy's UT build to use this though.
In the future it is planned to automatically download latest UT playable releases.

Double click on a server to join. It will ask for `UnrealTournament.exe` on Windows or `UnrealTournament` on Linux or Mac.

## Installation

For compiled binaries no installation is necessary. For source, see below:

### Arch Linux

UTLauncher is available in AUR as [utlauncher-git](https://aur.archlinux.org/packages/utlauncher-git/).  Installation is as follows:

```bash
curl -O https://aur.archlinux.org/packages/ut/utlauncher-git/utlauncher-git.tar.gz
tar -xf utlauncher-git.tar.gz
cd utlauncher-git
makepkg -s
pacman -U utlauncher-git*.pkg.tar.xz
```

### Build instructions

#### Requirements
You need to install Qt5 base development package, G++ compiler and cmake.

##### Ubuntu
```
sudo apt-add-repository ppa:ubuntu-sdk-team/ppa
sudo apt-get update
sudo apt-get install qtbase5-dev g++ cmake
```
##### openSUSE
`sudo zypper install libqt5-qtbase-devel gcc-c++ cmake`
##### openSUSE + Qt5 repo
`sudo zypper install libQt5Gui-devel libQt5Network-devel libQt5Widgets-devel gcc-c++ cmake`

### Build 
```
git clone https://github.com/CodeCharmLtd/UTLauncher.git
mkdir UTLauncher/build
cd UTLauncher
git submodule update --init --recursive
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

## License
Licensed under the MIT license

Copyright (c) 2014 Damian "Rush" Kaczmarek
from Code Charm Ltd
and other contributors

