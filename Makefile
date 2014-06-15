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

OBJS_COMMON = \
	$(OBJDIR)/action.o \
	$(OBJDIR)/cdefs.o \
	$(OBJDIR)/cell.o \
	$(OBJDIR)/clip.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/edit.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/gui.o \
	$(OBJDIR)/img.o \
	$(OBJDIR)/input.o \
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

CFLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -g -Isrc -I/usr/local/include `sdl-config --cflags`
LDFLAGS = -O2 -g $(LIBS_DIRS) $(LIBS_SDL) -lm
LDFLAGS_SERVER = -O2 -g $(LIBS_DIRS) $(LIBS_SDL) -lm

all: $(BINNAME) $(BINNAME_SERVER) dat/pal1.pal

clean:
	rm -r $(OBJDIR)

dat/pal1.pal: tools/buildpal.py $(IMAGES)
	python2 tools/buildpal.py

$(BINNAME): $(OBJDIR) $(OBJS_CLIENT)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS_CLIENT)

$(BINNAME_SERVER): $(OBJDIR) $(OBJS_SERVER)
	$(CC) -o $(BINNAME_SERVER) $(LDFLAGS_SERVER) $(OBJS_SERVER)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: all clean

