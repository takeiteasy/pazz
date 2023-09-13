ifeq ($(OS),Windows_NT)
	PROGEXT=.exe
	LIBEXT=.dll
else
	PROGEXT=
	UNAME:=$(shell uname -s)
	ifeq ($(UNAME),Darwin)
		LIBEXT=.dylib
	else
		LIBEXT=.so
	endif
endif

cli:
	$(CC) -Isrc src/spectre.c -o spectre$(PROGEXT)

lib:
	$(CC) -Isrc -shared -fpic -DNO_MAIN src/spectre.c -o libspectre$(LIBEXT)

app:
	$(CC) -Isrc -x objective-c -DNO_MAIN  src/spectre.c src/passe-gui.m -framework Cocoa -framework Security -o passe
	sh appify.sh -s passe -n Passe

all: lib app cli

install: cli
	mv spectre /usr/local/include

install-app: app
	mv Passe.app /Applications

default: cli

.PHONY: default lib cli
