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
	$(CC) -Isrc src/spectre.c -o build/spectre$(PROGEXT)

lib:
	$(CC) -Isrc -shared -fpic -DNO_MAIN src/spectre.c -o build/libspectre$(LIBEXT)

app:
	$(CC) -Isrc -x objective-c -DNO_MAIN  src/spectre.c passe.m -framework Cocoa -framework Security -o build/passe
	sh appify.sh -s build/passe -n Passe
	mv Passe.app build

default: cli

.PHONY: default lib cli
