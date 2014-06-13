#!/bin/sh
mingw32-windres parsniptheory.rc -O coff -o parsniptheory.res && \
gmake CC=mingw32-gcc OBJDIR=wbuild "CFLAGS=-Wall -Wextra -Wno-unused-parameter -O2 -g -Isrc -Iwinlibs -Iwinlibs/SDL" "LDFLAGS_SERVER=-O2 -g -Lwinlibs -lmingw32 SDL.dll SDL_net.dll -lSDLmain parsniptheory.res" "LDFLAGS=-O2 -g -Lwinlibs -lmingw32 SDL.dll SDL_net.dll -lSDLmain parsniptheory.res" BINNAME=parsniptheory-windows-i386.exe BINNAME_SERVER=parsnipserver-windows-i386.exe


