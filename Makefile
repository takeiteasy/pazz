ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=exe
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
	else
		LIBEXT=so
	endif
endif

default:
	$(CC) -shared -fpic -Ispectre spectre/spectre.c -o build/libspectre.$(LIBEXT)

.PHONY: default
