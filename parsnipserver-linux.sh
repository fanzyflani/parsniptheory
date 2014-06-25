#!/bin/sh
export ARCH=i386

case `uname -m` in
	armv6l)
		export ARCH=armv6l
		;;
	*)
		echo "Defaulting to i386."
esac

cd "$(dirname $0)"

export LD_PRELOAD=libs/linux-${ARCH}/libSDL-1.2.so.0.11.4:libs/linux-${ARCH}/libSDL_net-1.2.so.0.8.0:libs/linux-${ARCH}/libcaca.so.0:libs/linux-${ARCH}/libdirect-1.2.so.9:libs/linux-${ARCH}/libdirectfb-1.2.so.9:libs/linux-${ARCH}/libfusion-1.2.so.9:libs/linux-${ARCH}/libts-0.0.so.0
./parsnipserver-linux-${ARCH}

