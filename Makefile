ifeq ($(OS),Windows_NT)
	PROGEXT=.exe
else
	PROGEXT=
endif

default:
	$(CC) -Isrc -Ideps src/*.c -o passe$(PROGEXT)

.PHONY: default
