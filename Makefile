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

EMCC := `which emcc`
DST := static/pazz.js
SRC := src/pazz.c
LIB_DST := libpazz$(LIBEXT)

default: $(DST)

$(DST): $(SRC)
	$(EMCC) \
		-s SINGLE_FILE=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS="['_spectre', '_malloc', '_free']" \
		-s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8', 'UTF8ToString']" \
		$(SRC) -o $(DST)

web: $(DST)

$(LIB_DST): $(SRC)
	$(CC) -shared -fpic $(SRC) -o $(LIB_DST)

library: $(LIB_DST)

all: web library

clean:
	rm -f $(DST) $(LIB_DST)

test: library clean web

.PHONY: web clean library test
