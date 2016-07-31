#
# Trivial editor which handles UTF.
#

default: editor


editor: $(wildcard *.cc *.h)
	g++ -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -std=c++11 -ggdb $(wildcard *.cc) -o ../kilua   -I/usr/include/ncursesw -lncursesw -I/usr/include/lua5.2 -llua5.2

clean:
	rm -f editor *.orig core

.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 *.cc *.h
