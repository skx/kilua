#
# Makefile for kilua.
#
#


#
# Default target
#
all: kilua


#
#  Feature-flags:
#
FEATURES=
FEATURES+=-D_REGEXP=1
FEATURES+=-D_UNDO=1

#
# Version string.
#
FLAGS=-D_VERSION=\"0.4\"

#
#  The combined set of feature & version flags
#
OPT=$(FEATURES) $(FLAGS)


#
#  g++ will use a different set of flags.
#
ifeq ($(CC),g++)
   OPT+=--std=c++0x
else
   OPT+=-std=c99
endif


#
# Generate our embedded welcome-message.
#
welcome.h: util/embed welcome.txt
	perl util/embed --version=0.4 --array welcome.txt > welcome.h


#
# Generate our embedded configuration-file.
#
config.h: util/xxd kilua.lua
	perl util/xxd kilua.lua  > config.h


#
# Build the main binary.
#
kilua: Makefile $(wildcard *.c *.h) config.h welcome.h
	$(CC) ${OPT} -o kilua -ggdb $(wildcard *.c) -Wall -Wextra -Werror -W -pedantic $(shell pkg-config --cflags --libs lua5.2)


#
# Reformat our code
#
.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 *.c *.h


#
# Cleanup
#
.PHONY: indent
clean:
	rm -rf kilua *.orig core valgrind.out kilua.dSYM welcome.h config.h

#
#  Run our binary under valgrind.
#
valgrind: kilua
	valgrind --log-file=valgrind.out --read-var-info=yes --read-inline-info=yes --undef-value-errors=yes --track-origins=yes --keep-stacktraces=alloc-and-free  --tool=memcheck --leak-check=yes  --track-origins=yes -v --leak-check=full --show-leak-kinds=all -v ./kilua ./t
