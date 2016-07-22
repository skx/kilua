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
# Build the main binary.
#
kilua: Makefile $(wildcard *.c *.h)
	$(CC) ${FEATURES} ${FLAGS} -o kilua -ggdb $(wildcard *.c) -Wall -Wextra -Werror -W -pedantic -std=c99 $(shell pkg-config --cflags --libs lua5.2)


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
	rm -rf kilua *.orig core valgrind.out kilua.dSYM

#
#  Run our binary under valgrind.
#
valgrind: kilua
	valgrind --log-file=valgrind.out --read-var-info=yes --read-inline-info=yes --undef-value-errors=yes --track-origins=yes --keep-stacktraces=alloc-and-free  --tool=memcheck --leak-check=yes  --track-origins=yes -v --leak-check=full --show-leak-kinds=all -v ./kilua ./t
