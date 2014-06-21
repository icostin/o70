o70
===

Ostropel scripting engine

The project products are a library (that does not depend on libc, just c42 
which is freestanding) and a command line tool (which depends on c42, c42svc,
c42clia and o70 library).


Compiling
====
Prerequisites: 
* gcc compatible C compiler (gcc/clang)
* GNU make

Sources:
  git clone git@github.com:icostin/c42.git
  git clone git@github.com:icostin/c42svc.git
  git clone git@github.com:icostin/c42clia.git
  git clone git@github.com:icostin/o70.git

Building:
  make -C c42 install [CC=PATH_TO_COMPILER] [CFG=debug|release]
  make -C c42clia install [CC=PATH_TO_COMPILER] [CFG=debug|release]
  make -C c42svc install [CC=PATH_TO_COMPILER] [CFG=debug|release]
  make -C o70 install [CC=PATH_TO_COMPILER] [CFG=debug|release]

The files are built somewhere under: /tmp/$USER-build/
and by default they are installed under ./targets/$TARGET-$CFG

For instance to build for Android arm one needs the Android NDK and
runs:
  make -C c42 install CC=$NDK/bin/arm-linux-androideabi-gcc
  make -C c42clia install CC=$NDK/bin/arm-linux-androideabi-gcc
  make -C c42svc install CC=$NDK/bin/arm-linux-androideabi-gcc
  make -C o70 install CC=$NDK/bin/arm-linux-androideabi-gcc
and obtains:
  ./targets/arm-linux-androideabi-release/lib/libc42.so
  ./targets/arm-linux-androideabi-release/lib/libc42svc.so
  ./targets/arm-linux-androideabi-release/lib/libo70.so
  ./targets/arm-linux-androideabi-release/bin/o70   (dynamically linked)
  ./targets/arm-linux-androideabi-release/bin/o70s  (static linked)

