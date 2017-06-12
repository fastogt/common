About Common
===============
[![Travis Build Status](https://travis-ci.org/fastogt/common.svg?branch=master)](https://travis-ci.org/fastogt/common)

FastoGT common sources

Visit our site: [www.fastogt.com](http://www.fastogt.com)

Contribute
==========
Contributions are always welcome! Just try to follow our coding style: [FastoGT Coding Style](https://github.com/fasto/common/wiki/Coding-Style)

Build
========

Windows
-------
cmake .. -GNinja -DQT_ENABLED=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/mingw64
cmake .. -GNinja -DQT_ENABLED=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/mingw32

Linux, FreeBSD, MacOS X
-------
cmake .. -GNinja -DQT_ENABLED=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local

Android
-------
cmake .. -GNinja -DQT_ENABLED=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/opt/android-ndk/platforms/android-9/arch-arm/usr/

License
=======

Copyright (C) 2014-2016 FastoGT (http://www.fastogt.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

**Style**<br/>
.clang_format
cmake -DCHECK_STYLE=ON
make check_style
Note: needed clang-tidy
