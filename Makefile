all: kilo


kilo: kilo.c
	$(CC) -o kilo -ggdb kilo.c -Wall -Wextra -Werror -W -pedantic -std=c99 $(shell pkg-config --cflags --libs lua5.2)

.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 kilo.c


protos: kilo.c
	cproto kilo.c $(shell pkg-config --cflags lua5.2) > kilo.h
clean:
	rm kilo || true
