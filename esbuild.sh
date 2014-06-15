#!/bin/sh

export EMCC_FAST_COMPILER=0

export SRCDIR="src"

export OBJS_COMMON="\
	${SRCDIR}/action.c \
	${SRCDIR}/cdefs.c \
	${SRCDIR}/cell.c \
	${SRCDIR}/clip.c \
	${SRCDIR}/draw.c \
	${SRCDIR}/edit.c \
	${SRCDIR}/game.c \
	${SRCDIR}/gui.c \
	${SRCDIR}/img.c \
	${SRCDIR}/input.c \
	${SRCDIR}/obj.c \
	${SRCDIR}/screen.c \
	${SRCDIR}/server.c \
	${SRCDIR}/shared.c \
	${SRCDIR}/team.c \
	${SRCDIR}/tools.c"
	#

export OBJS="\
	${OBJS_COMMON} \
	${SRCDIR}/main.c \
	${SRCDIR}/smain.c"
	#

export OBJS_CLIENT="\
	${OBJS_COMMON} \
	${SRCDIR}/main.c"
	#

export OBJS_SERVER="\
	${OBJS_COMMON} \
	${SRCDIR}/smain.c"
	#

/usr/home/ben/Downloads/emscripten/emcc -o index.js -O2 -DNO_NET $OBJS_CLIENT -Isackit $(for A in dat/* tga/*; do echo --preload-file "${A}"; done)

