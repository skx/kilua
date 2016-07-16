all: kilo

kilo: kilo.c
	$(CC) -o kilo -ggdb kilo.c -Wall -Wextra -Werror -W -pedantic -std=c99 $(shell pkg-config --cflags --libs lua5.2)

clean:
	rm kilo || true
