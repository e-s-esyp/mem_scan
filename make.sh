#!/bin/bash

NDK_PATH=/home/joseph/Android/Sdk/ndk/23.1.7779620
SYSROOT=$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CLANG=$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin/clang
LIB_PATH=$SYSROOT/usr/lib/arm-linux-androideabi/29  # Используем найденный API level

$CLANG --target=armv7a-linux-androideabi \
    --sysroot=$SYSROOT \
    -L$LIB_PATH \
    -o b main32.c -fPIE -pie -Wl,-dynamic-linker,/system/bin/linker 
#    -v
#    $LIB_PATH/crtbegin_dynamic.o \
#    $LIB_PATH/crtend_android.o \

adb push b /data/local/tmp/memscan
