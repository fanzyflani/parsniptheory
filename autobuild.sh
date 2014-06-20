#!/bin/sh
rm -r dist
mkdir dist && \
cp *.txt *.dll dist/ && \
./winbuild.sh && \
gmake CC="cc -m32" LIBS_DIRS="-L/compat/i386/usr/local/lib -laa" LIBS_SACKIT="sackit/freebsd-i386/*.o" OBJDIR="build/freebsd-i386"  BINNAME="parsniptheory-freebsd-i386"  BINNAME_SERVER="parsnipserver-freebsd-i386"  && \
gmake CC="cc -m64" LIBS_SACKIT="sackit/freebsd-amd64/*.o" OBJDIR="build/freebsd-amd64" BINNAME="parsniptheory-freebsd-amd64" BINNAME_SERVER="parsnipserver-freebsd-amd64" && \
true gmake CC="/compat/linux/usr/bin/gcc -m32" LIBS_DIRS="-nostdlib -nostartfiles -Wl,-rpath,libs/i386-linux/ -L/compat/linux/usr/lib winlibs/libgcc-linux/*" LIBS_ZLIB="" LIBS_SDL="libs/linux-i386/*" LIBS_SACKIT="sackit/linux-i386/*.o" OBJDIR="build/linux-i386"  BINNAME="parsniptheory-linux-i386"  BINNAME_SERVER="parsnipserver-linux-i386"  && \
true brandelf -t linux parsniptheory-linux-i386 && \
true brandelf -t linux parsnipserver-linux-i386 && \
strip parsniptheory-freebsd-i386 parsniptheory-freebsd-amd64 parsniptheory-windows-i386.exe parsniptheory-linux-i386 && \
strip parsnipserver-freebsd-i386 parsnipserver-freebsd-amd64 parsnipserver-windows-i386.exe parsnipserver-linux-i386 && \
cp parsniptheory-freebsd-i386 parsniptheory-freebsd-amd64 parsniptheory-windows-i386.exe parsniptheory-linux-i386 dist/ && \
cp parsnipserver-freebsd-i386 parsnipserver-freebsd-amd64 parsnipserver-windows-i386.exe parsnipserver-linux-i386 dist/ && \
cp parsnip*-linux.sh dist/ && \
cp -R dat lvl lic libs dist/ && \
echo "DONE"

