all: kilo

FLAGS=-D_REGEXP=1 -D_UNDO=1 -DKILO_VERSION=\"0.3\"

kilo: kilo.c Makefile kilo.h undo_stack.h
	$(CC) ${FLAGS} -o kilo -ggdb kilo.c -Wall -Wextra -Werror -W -pedantic -std=c99 $(shell pkg-config --cflags --libs lua5.2)

.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 *.c *.h


protos: kilo.c
	cproto kilo.c $(shell pkg-config --cflags lua5.2) > kilo.h
clean:
	rm kilo || true

valgrind:
	make clean
	make
	valgrind --log-file=valgrind.out --read-var-info=yes --read-inline-info=yes --undef-value-errors=yes --track-origins=yes --keep-stacktraces=alloc-and-free  --tool=memcheck --leak-check=yes  --track-origins=yes -v --leak-check=full --show-leak-kinds=all -v ./kilo /dev/null
