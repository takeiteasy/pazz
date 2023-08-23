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

default: cli

all: lib cli

.PHONY: default all lib cli
