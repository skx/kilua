#
# Makefile for kilua.
#
#


#
# Default target
#
all: kilua

#
# Generate our embedded configuration-file.
#
src/config.h: util/xxd kilua.lua
	perl util/xxd kilua.lua  > src/config.h


#
# Build the main binary.
#
kilua: src/config.h $(wildcard src/*.cc src/*.h)
	cd src && make


#
# Reformat our code
#
.PHONY: indent
indent:
	cd src && make indent


#
# Cleanup
#
.PHONY: indent
clean:
	cd src && make clean
	rm -f kilua src/config.h
