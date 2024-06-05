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

default: all

lib:
	$(CC) -Isrc -shared -fpic src/spectre.c -o libspectre$(LIBEXT)

cli: lib
	$(CC) -Isrc src/cli.c -L. -lspectre -o spectre$(PROGEXT)

all: lib cli

.PHONY: default all cli lib
