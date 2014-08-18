## UTLauncher

UTLauncher is a standalone application to join UT4 servers. You will need latest raxxy's UT build to use this though.
In the future it is planned to automatically download latest UT playable releases.

Double click on a server to join. It will ask for UnrealTournament.exe

## Installation

### Arch Linux

UTLauncher is available in AUR as [utlauncher-git](https://aur.archlinux.org/packages/utlauncher-git/).  Installation is as follows:

```bash
curl -O https://aur.archlinux.org/packages/ut/utlauncher-git/utlauncher-git.tar.gz
tar -xf utlauncher-git.tar.gz
cd utlauncher-git
makepkg -s
pacman -U utlauncher-git*.pkg.tar.xz
```

## License
Licensed under the MIT license

Copyright (c) 2014 Damian "Rush" Kaczmarek
from Code Charm Ltd
and other contributors

