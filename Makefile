# TODO: BSD make compatibility
# (use gmake)
OBJDIR = build
SRCDIR = src
BINNAME = parsniptheory

IMAGES = \
	tga/player.tga \
	tga/tiles1.tga

OBJS = \
	$(OBJDIR)/cdefs.o \
	$(OBJDIR)/cell.o \
	$(OBJDIR)/clip.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/edit.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/img.o \
	$(OBJDIR)/input.o \
	$(OBJDIR)/obj.o \
	$(OBJDIR)/screen.o \
	$(OBJDIR)/team.o \
	$(OBJDIR)/tools.o \
	\
	$(OBJDIR)/main.o
	#

INCLUDES = src/common.h src/obj.h

CFLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -g -Isrc -I/usr/local/include `sdl-config --cflags`
LDFLAGS = -O2 -g $(LIBS_DIRS) $(LIBS_SDL) -lm
LIBS_DIRS = -L/usr/local/lib
LIBS_SDL = `sdl-config --libs`

all: $(BINNAME) dat/pal1.pal

clean:
	rm -r $(OBJDIR)

dat/pal1.pal: tools/buildpal.py $(IMAGES)
	python2 tools/buildpal.py

$(BINNAME): $(OBJDIR) $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: all clean

