#cRTED TED-music player's makefile

VERSION=1.0

CC=gcc
AR=ar rcs
CP=cp
RM=rm -f

APPNAME = crted
APPSOURCE = cRTED.c
LIBHEADER = libcRTED.h
LIBSOURCE = libcRTED.c
LIBSOURCES = $(LIBHEADER) $(LIBSOURCE)
SUBSOURCES = CPlus4/*.c host/*.c GUI/*.c resources/*.h
LIBSHARED = libcRTED.so
LIBSTATIC = libcRTED.a
APPDIR = /usr/bin
LIBDIR = /usr/lib
INCDIR = /usr/include
DEPENDS=libsdl1.2debian

CRTED_PLATFORM_PC = 1
CRTED_PLATFORM = $(CRTED_PLATFORM_PC)
BACKEND_IDS = -DCRTED_PLATFORM=$(CRTED_PLATFORM_PC)

ifeq ($(CRTED_PLATFORM),$(CRTED_PLATFORM_PC))
DEFINES = -DCRTED_PLATFORM_PC -DVERSION=\"$(VERSION)\"
CCFLAGS = `sdl-config --cflags --libs`  -lSDL -lm  -O2  -s
endif


all: $(APPNAME) $(LIBSHARED) $(LIBSTATIC) package $(APPNAME).exe $(APPNAME).com


$(APPNAME): $(APPSOURCE) $(LIBSOURCES) $(SUBSOURCES) builtin
	$(CC)  $(APPSOURCE)  -o crted $(DEFINES) -DLINUX  $(CCFLAGS)


$(LIBSHARED): $(LIBSOURCES) $(SUBSOURCES)
	$(CC)  -c $(LIBSOURCE)  -fpic  -o $(LIBSHARED).o  $(DEFINES)  -DLIBRARY  $(CCFLAGS)
	$(CC)  $(LIBSHARED).o  -shared  -o $(LIBSHARED)
	$(RM) $(LIBSHARED).o


$(LIBSTATIC): $(LIBSOURCES) $(SUBSOURCES)
	$(CC)  -c $(LIBSOURCE)  -o $(LIBSTATIC).o  $(DEFINES)  -DLIBRARY  $(CCFLAGS)
	$(AR) $(LIBSTATIC) $(LIBSTATIC).o
	$(RM) $(LIBSTATIC).o


test: $(APPNAME)
	./$(APPNAME) MUSIC/TMF resources/builtin-music.ted 5 default-playlist.tel -info

testcli: $(APPNAME)
	./$(APPNAME) -cli -info MUSIC/TMF resources/builtin-music.ted 5 default-playlist.tel # -autoexit


font:
	gcc GUI/fontgen.c -o GUI/fontgen -lm
	GUI/fontgen

builtin:
	gcc  resources/tools/bin2array.c  -o resources/tools/bin2array
	resources/tools/bin2array resources/builtin-music.ted

#playlist:
#	find MUSIC -type f > default-playlist.tel


package:
	mkdir -p package/usr/bin package/usr/share/icons package/usr/share/pixmaps \
             package/usr/share/applications package/usr/share/mime/packages package/DEBIAN
	echo "Package: crted\nVersion: $(VERSION) \nSection: custom\nPriority: optional\nArchitecture: \
          all\nEssential: no\nInstalled-Size: 180\nDepends: $(DEPENDS)\nMaintainer: \
          Hermit\nDescription: cRTED CPlus4 TED-music player" \
          > package/DEBIAN/control
	cp -a crted crted.sh crteda package/usr/bin/
	cp -a resources/cRTED-icon.png  package/usr/share/icons/
	cp -a resources/cRTED-icon.png  package/usr/share/pixmaps/
	cp -a resources/x-cRTED-extension-ted.xml resources/x-cRTED-extension-tmf.xml \
          resources/x-cRTED-extension-tel.xml  package/usr/share/mime/packages/
	cp -a resources/cRTED.desktop  package/usr/share/applications/
	chmod 777 package/usr/bin/crted package/usr/bin/crteda package/usr/bin/crted.sh
	dpkg-deb --build package && mv package.deb crted-$(VERSION).deb
	rm -r -f package



install: $(APPNAME) $(LIBSHARED) $(LIBSTATIC)  # package
	dpkg -r crted && dpkg -i crted-$(VERSION).deb
#	$(CP)  $(LIBSHARED) $(LIBSTATIC)  $(LIBDIR)
#	$(CP)  $(LIBHEADER)  $(INCDIR)
#	$(CP)  $(APPNAME) crteda crted.sh  $(APPDIR)


uninstall:
	dpkg -r crted
	$(RM) $(APPDIR)/$(APPNAME) $(APPDIR)/crteda $(APPDIR)/crted.sh \
          $(LIBDIR)/$(LIBSHARED) $(LIBDIR)/$(LIBSTATIC) $(INCDIR)/$(LIBHEADER)


ico:
	icotool  -c  -o resources/cRTED-icon.ico  resources/cRTED-icon.png
	i686-w64-mingw32-windres -O coff -i resources/cRTED-icon.rc -o resources/cRTED-icon.res


$(APPNAME).exe: $(APPSOURCE) $(LIBSOURCES) $(SUBSOURCES) builtin
	i686-w64-mingw32-gcc  $(APPSOURCE)  resources/cRTED-icon.res  -o $(APPNAME).exe  $(DEFINES)  -DWINDOWS  \
    -s  -O2  -lmingw32  -mwindows  -lSDLmain  -lSDL  \
    -I/usr/local/sdlmingw/include/SDL  -L/usr/local/sdlmingw/lib  #  -Wl,-subsystem,console
	sed -i 's/stdout.txt/\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0/' $(APPNAME).exe
	sed -i 's/stderr.txt/\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0/' $(APPNAME).exe

$(APPNAME).com: $(APPSOURCE) $(LIBSOURCES) $(SUBSOURCES) builtin
	i686-w64-mingw32-gcc  $(APPSOURCE)  resources/cRTED-icon.res  -o $(APPNAME).com  $(DEFINES)  -DWINDOWS  \
    -s  -O2  -Wl,-subsystem,console  -lmingw32  -lSDLmain  -lSDL  \
    -I/usr/local/sdlmingw/include/SDL  -L/usr/local/sdlmingw/lib
	sed -i 's/stdout.txt/\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0/' $(APPNAME).com
	sed -i 's/stderr.txt/\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0/' $(APPNAME).com

testw: crted.exe
	wine crted.exe # resources/builtin-music.ted -autoexit # -cli

testwcli: crted.com
	wine crted.com # resources/builtin-music.ted -autoexit # -cli


clean:
	$(RM) $(APPNAME) $(APPNAME).exe $(APPNAME).com *.o *.a *.so *.deb package # GUI/graphics/fonts*.h


purge: clean uninstall
	$(RM) *.deb
