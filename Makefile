EMCC := `which emcc`
DST := web/pazz.js
SRC := src/pazz.c

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

default: $(DST)

$(DST): $(SRC)
	$(EMCC) \
		-s SINGLE_FILE=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS="['_spectre', '_malloc', '_free']" \
		-s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8', 'UTF8ToString']" \
		$(SRC) -o $(DST)

web: $(DST)

CLI_SRC := src/pazz_cli.c $(SRC)
CLI_DST := pazz$(PROGEXT)

$(CLI_DST): $(CLI_SRC)
	$(CC) -Isrc $(CLI_SRC) -o $(CLI_DST)

cli: $(CLI_DST)

LIB_DST := libpazz$(LIBEXT)

$(LIB_DST): $(CLI_SRC)
	$(CC) -shared -fpic $(CLI_SRC) -o $(LIB_DST)

library: $(LIB_DST)

all: web cli library

.PHONY: web clean

clean:
	rm -f $(DST) $(CLI_DST) $(LIB_DST)
