#
# Trivial editor which handles UTF.
#

default: editor

#
# Generate our embedded configuration-file.
#
editor_conf.h: util/xxd editor.lua
	perl util/xxd editor.lua  > editor_conf.h

editor: $(wildcard *.cc *.h)
	g++ -std=c++11 -ggdb $(wildcard *.cc) -o editor   -I/usr/include/ncursesw -lncursesw -I/usr/include/lua5.2 -llua5.2 -ldl

clean:
	rm -f editor *.orig core

.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 *.cc *.h
