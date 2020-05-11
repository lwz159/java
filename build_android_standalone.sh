#!/bin/sh
C:/Users/lwz15/AppData/Local/Android/sdk/ndk/17.2.4988734/build/tools/my-toolchain/bin/make clean
export NDK=C:/Users/lwz15/AppData/Local/Android/sdk/ndk/17.2.4988734/build/tools/my-toolchain

# Tell configure what tools to use.
target_host=arm-linux-androideabi
export AR=$target_host-gcc-ar
export AS=$target_host-gcc
export CC=$target_host-gcc
export CXX=$target_host-g++
export CPP="${target_host}-gcc -E"
export LD=$target_host-ld
export STRIP=$target_host-strip

export SYSROOT=$NDK/sysroot

export CPPFLAGS="-I${SYSROOT}/usr/include"
export CPU=arm
export PREFIX=C:/Users/lwz15/Downloads/libevent-2.1.11-stable/libevent-2.1.11-stable/$CPU
export ADDI_CFLAGS="-marm"
./configure --host=arm-linux-androideabi --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX
C:/Users/lwz15/AppData/Local/Android/sdk/ndk/17.2.4988734/build/tools/my-toolchain/bin/make clean
C:/Users/lwz15/AppData/Local/Android/sdk/ndk/17.2.4988734/build/tools/my-toolchain/bin/make
C:/Users/lwz15/AppData/Local/Android/sdk/ndk/17.2.4988734/build/tools/my-toolchain/bin/make install