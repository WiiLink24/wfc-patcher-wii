# WWFC Downloader Payload
Referred to internally as the 'stage 1' payload, the downloader payload is an intermittent game-independent payload.
It's responsible for downloading the 'stage 2' (final) payload and verifying its RSA-SHA-256 signature before launching
it.

The stage 0 code verifies the stage 1 payload against a specific hash, preventing it from being updated. So essentially
it's an extension of the stage 0 payload, but stored on the server for lack of a good place to store it in game memory.

## Building
The code is very small and as such does not need a very complicated build system. The Python script `make-stage1.py`
includes the commands to build it. You will need the `DEVKITPPC` environment variable set.

## Other Licenses
Software released under other licenses are used:
* [FIPS 180-2 SHA-256 implementation](https://github.com/ogay/sha2) - Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
* [RSA Verify implementation](https://github.com/jhallen/rsa-verify) - Copyright (C) 2010 The Chromium OS Authors

The rest of the code is licensed under the same custom BSD-style license as the rest of the respository. The full text can be found in the [LICENSE](../LICENSE) file.
