About Common
===============
[![Travis Build Status](https://travis-ci.org/fastogt/common.svg?branch=master)](https://travis-ci.org/fastogt/common)
[![Coverage Status](https://coveralls.io/repos/github/fastogt/common/badge.svg?branch=master)](https://coveralls.io/github/fastogt/common?branch=master)

FastoGT common sources

Visit our site: [fastogt.com](https://fastogt.com)

Contribute
==========
Contributions are always welcome! Just try to follow our coding style: [FastoGT Coding Style](https://github.com/fastogt/fastonosql/wiki/Coding-Style)

Build
========

Dependencies (Optional)
-------
  * Zlib
  * BZip2
  * Snappy
  * LZ4
  * Json-c
  * LibEv
  * Qt

Windows:
-------
`cmake .. -GNinja -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/mingw64`<br>
`cmake .. -GNinja -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/mingw32`

Linux, FreeBSD, MacOSX:
-------
`cmake .. -GNinja -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local`

Android:
-------
### Enviroment:
`ANDROID_PLATFORM=android-16`<br>
`ANDROID_NDK=/home/sasha/Android/Sdk/ndk-bundle`

`
cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE -DANDROID_PLATFORM=$ANDROID_PLATFORM -DCMAKE_INSTALL_PREFIX=$ANDROID_NDK/platforms/$ANDROID_PLATFORM/arch-arm/usr/
`

IOS:
-------
`
cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=${CMAKE_OSX_SYSROOT}/usr
`

License
=======

Copyright (C) 2014-2020 FastoGT (https://fastogt.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
