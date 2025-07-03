EMCC := `which emcc`

default:
	$(EMCC) \
		-s SINGLE_FILE=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS="['_generate', '_malloc', '_free']" \
		-s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8', 'UTF8ToString']" \
		pazz.c -o web/pazz.js
