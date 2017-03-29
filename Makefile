VERSION=0.0.1

CC=gcc
CFLAGS=-Wall -Wno-pointer-sign -g
INSTALL=install
PKGCONFIG=pkg-config
XML2CONFIG=xml2-config

# prefix = $(HOME)
prefix = $(DESTDIR)/usr
BINDIR = $(prefix)/bin
DATADIR = $(prefix)/share
DESKTOPDIR = $(DATADIR)/applications
ICONPATH = $(DATADIR)/icons/hicolor
ICONDIR = $(ICONPATH)/scalable/apps
MANDIR = $(DATADIR)/man/man1
XSLTDIR = $(DATADIR)/picotools/xslt
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(ICONPATH)

NAME = pico
ICONFILE = $(NAME).svg
DESKTOPFILE = $(NAME).desktop
MANFILES = $(NAME).1

MACOSXINSTALL = /Applications/Pico.app
MACOSXFILES = packaging/macosx

# find libps5000
libdc-local := $(wildcard /opt/picoscope/lib/libps5000.la)
libdc-local64 := $(wildcard /opt/picoscope/lib/libps5000.la)
libdc-usr := $(wildcard /opt/picoscope/lib/libps5000.la)
libdc-usr64 := $(wildcard /opt/picoscope/lib/libps5000.la)

ifneq ($(strip $(libdc-local)),)
        LIBDIR = /usr/local
        LIBINCLUDES = -I$(LIBDIR)/include/libps5000-1.5
        LIBARCHIVE = $(LIBDIR)/lib/libps5000.la
else ifneq ($(strip $(libdc-local64)),)
        LIBDIR = /usr/local
        LIBINCLUDES = -I$(LIBDIR)/include/libps5000-1.5
        LIBARCHIVE = $(LIBDIR)/lib/libps5000.la
else ifneq ($(strip $(libdc-usr)),)
        LIBDIR = /usr
        LIBINCLUDES = -I$(LIBDIR)/include/libps5000-1.5
        LIBARCHIVE = $(LIBDIR)/lib/libps5000.la
else ifneq ($(strip $(libdc-usr64)),)
        LIBDIR = /usr
        LIBINCLUDES = -I$(LIBDIR)/include/libps5000-1.5
        LIBARCHIVE = $(LIBDIR)/lib/libps5000.la
else
#$(error Cannot find libps5000 - please edit Makefile)
		LIBINCLUDES = -I/opt/picoscope//include
		#LIBARCHIVE = /opt/picoscope/lib/libps5000.la
endif

# Get libusb if it exists, but don't complain about it if it doesn't.
LIBUSB = $(shell $(PKGCONFIG) --libs libusb-1.0 2> /dev/null)
LIBGTK = $(shell $(PKGCONFIG) --libs gtk+-2.0 glib-2.0 gconf-2.0 libglade-2.0)
LIBSDL = $(shell $(PKGCONFIG) --libs sdl) -lSDL_gfx -lSDL_ttf
LIBCFLAGS = $(LIBINCLUDES) $(shell $(PKGCONFIG) --cflags sdl)
LIBPS = -L/opt/picoscope/lib -lps5000 $(LIBUSB)

LIBS = $(LIBSDL) $(LIBXML2) $(LIBGTK) $(LIBPS) -lpthread

OBJS = main.o scope.o handlers.o wview/wview.o wview/sdl_display.o wview/scrollbar.o $(RESFILE)

$(NAME): $(OBJS)
	$(CC) -rdynamic $(LIBINCLUDES) $(LDFLAGS) -o $(NAME) $(OBJS) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) $(LIBINCLUDES) $(GLADECFLAGS) $(GTK2CFLAGS) $(GLIB2CFLAGS) $(GCONF2CFLAGS) -c main.c

handlers.o: handlers.c
	$(CC) $(CFLAGS) $(LIBINCLUDES) $(GLADECFLAGS) $(GTK2CFLAGS) $(GLIB2CFLAGS) $(GCONF2CFLAGS) -c handlers.c

scope.o: scope.c
	$(CC) $(CFLAGS) $(LIBINCLUDES) $(GLADECFLAGS) $(GTK2CFLAGS) $(GLIB2CFLAGS) $(GCONF2CFLAGS) -c scope.c

install: $(NAME)
	$(INSTALL) -d -m 755 $(BINDIR)
	$(INSTALL) $(NAME) $(BINDIR)
	$(INSTALL) -d -m 755 $(DESKTOPDIR)
	$(INSTALL) $(DESKTOPFILE) $(DESKTOPDIR)
	$(INSTALL) -d -m 755 $(ICONDIR)
	$(INSTALL) $(ICONFILE) $(ICONDIR)
	@-if test -z "$(DESTDIR)"; then \
		$(gtk_update_icon_cache); \
	fi
	$(INSTALL) -d -m 755 $(MANDIR)
	$(INSTALL) -m 644 $(MANFILES) $(MANDIR)
	@-if test ! -z "$(XSLT)"; then \
		$(INSTALL) -d -m 755 $(DATADIR)/subsurface; \
		$(INSTALL) -d -m 755 $(XSLTDIR); \
		$(INSTALL) -m 644 $(XSLTFILES) $(XSLTDIR); \
	fi

LIBXML2 = $(shell $(XML2CONFIG) --libs)
XML2CFLAGS = $(shell $(XML2CONFIG) --cflags)
GLIB2CFLAGS = $(shell $(PKGCONFIG) --cflags glib-2.0)
GCONF2CFLAGS =  $(shell $(PKGCONFIG) --cflags gconf-2.0)
GTK2CFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-2.0)
GLADECFLAGS = $(shell $(PKGCONFIG) --cflags libglade-2.0)

install-macosx: $(NAME)
	$(INSTALL) -d -m 755 $(MACOSXINSTALL)/Contents/Resources
	$(INSTALL) -d -m 755 $(MACOSXINSTALL)/Contents/MacOS
	$(INSTALL) $(NAME) $(MACOSXINSTALL)/Contents/MacOS/
	$(INSTALL) $(MACOSXFILES)/pico.sh $(MACOSXINSTALL)/Contents/MacOS/
	$(INSTALL) $(MACOSXFILES)/PkgInfo $(MACOSXINSTALL)/Contents/
	$(INSTALL) $(MACOSXFILES)/Info.plist $(MACOSXINSTALL)/Contents/
	$(INSTALL) $(ICONFILE) $(MACOSXINSTALL)/Contents/Resources/
	$(INSTALL) $(MACOSXFILES)/Pico.icns $(MACOSXINSTALL)/Contents/Resources/

clean:
	rm -f $(OBJS) *~ $(NAME)

#znooh:
#	gcc -rdynamic -lps5000 -lSDL -lSDL_gfx -lSDL_ttf -Wall -g -o pico main.c handlers.c scope.c wview/wview.c wview/sdl_display.c wview/scrollbar.c `pkg-config --cflags --libs libglade-2.0 gthread-2.0`
