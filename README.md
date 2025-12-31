# WiiLink WFC Patcher
WiiLink Wi-Fi Connection aims to be an open source server replacement for Nintendo Wi-Fi Connection.
This repository includes the client side patches necessary for connecting to WWFC.

The repository is split into the following directories:

* [patch](patch) - The initial 'stage 0' patch made to the game to connect to WWFC.
* [stage1](stage1) - The non-game-specific stage 1 payload downloaded by the initial patch.
* [payload](payload) - The final stage 2 payload downloaded and verified by stage 1. This is a game-specific payload
that makes patches to the running game, including important security patches and general improvements to the game's code.

Each directory includes its own README file with more detailed information on the section.

# Building

You need:

- Python 3.11+ with the `cryptography` module
- devkitPPC
- A domain name under which your WWFC server will be reachable (`${DOMAIN}`)

To build everything, run the make script:

```sh
./make.sh --all -- -j8 -DWWFC_DOMAIN=\"${DOMAIN}\"
```

The script will create a directory `dist/` which contains the payload files necessary to run the server. Copy those files into a `payload/` directory located next to your `wwfc` binary.

The gecko codes will be located in the `patch/build/` directory as `.txt` files.

# License
Unless a file explicitly states otherwise, everything in this repository is licensed under a custom BSD-style
license detailed in the [LICENSE](LICENSE) file. The purpose of this license is to allow easy integration into
permissively licensed projects, while requiring strict attribution to discourage implementation into the existing
closed-source, proprietary environment.

The above license may not be compatible with copyleft licenses such as the GPL. For this reason, this software
is also made available under the GNU General Public License version 2 (or any later version). You may choose the terms
of the above license or the GPL, but not both. A copy of the GPL version 2 is available online at:
https://www.gnu.org/licenses/gpl-2.0
