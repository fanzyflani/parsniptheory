# TODO: BSD make compatibility
# (use gmake)
OBJDIR = build
SRCDIR = src
BINNAME = parsniptheory
BINNAME_SERVER = parsnipserver

IMAGES = \
	tga/food1.tga \
	tga/player.tga \
	tga/tiles1.tga

IMAGES_PNG = \
	dat/font16.img \
	dat/font57.img \
	dat/fontnum1.img \
	dat/food1.img \
	dat/icons1.img \
	dat/player.img \
	dat/tiles1.img

OBJS_COMMON = \
	$(OBJDIR)/action.o \
	$(OBJDIR)/audio.o \
	$(OBJDIR)/cdefs.o \
	$(OBJDIR)/cell.o \
	$(OBJDIR)/clip.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/edit.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/gui.o \
	$(OBJDIR)/img.o \
	$(OBJDIR)/input.o \
	$(OBJDIR)/network.o \
	$(OBJDIR)/obj.o \
	$(OBJDIR)/screen.o \
	$(OBJDIR)/server.o \
	$(OBJDIR)/shared.o \
	$(OBJDIR)/team.o \
	$(OBJDIR)/tools.o
	#

OBJS = \
	$(OBJS_COMMON) \
	$(OBJDIR)/main.o \
	$(OBJDIR)/smain.o
	#

OBJS_CLIENT = \
	$(OBJS_COMMON) \
	$(OBJDIR)/main.o
	#

OBJS_SERVER = \
	$(OBJS_COMMON) \
	$(OBJDIR)/smain.o
	#

INCLUDES = src/common.h src/obj.h

LIBS_DIRS = -L/usr/local/lib
LIBS_SDL = `sdl-config --libs` -lSDL_net
LIBS_ZLIB = -lz
LIBS_SACKIT = sackit/*.o

CFLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -g -Isackit -Isrc -I/usr/local/include `sdl-config --cflags`
LDFLAGS = -O2 -g $(LIBS_DIRS) $(LIBS_SDL) $(LIBS_ZLIB) $(LIBS_SACKIT) -lm
LDFLAGS_SERVER = -O2 -g $(LIBS_DIRS) $(LIBS_SDL) $(LIBS_ZLIB) $(LIBS_SACKIT) -lm

all: $(BINNAME) $(BINNAME_SERVER) dat/pal1.pal $(IMAGES_PNG)

clean:
	rm -r $(OBJDIR)

dat/pal1.pal: tools/buildpal.py $(IMAGES)
	python2 tools/buildpal.py

$(BINNAME): $(OBJDIR) $(OBJS_CLIENT)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS_CLIENT)

$(BINNAME_SERVER): $(OBJDIR) $(OBJS_SERVER)
	$(CC) -o $(BINNAME_SERVER) $(LDFLAGS_SERVER) $(OBJS_SERVER)

dat/%.img: tga/%.tga
	python2 tools/tga2png.py $< $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: all clean

