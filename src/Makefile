#
# Compilation flags and libraries we use.
#
CPPFLAGS+=-fsanitize=address -fno-omit-frame-pointer -std=c++11 -ggdb -Wall -Werror -I/usr/include/ncursesw -lncursesw -I/usr/include/lua5.2 -llua5.2 -DKILUA_VERSION="\"0.5\""
LDLIBS+=$(shell pkg-config --libs ncursesw) $(shell pkg-config --libs lua5.2)  -fsanitize=address -fno-omit-frame-pointer -lstdc++

#
# The linker & objects.
#
LINKER=$(CC) -o
SOURCES := $(wildcard *.cc)
OBJECTS := $(SOURCES:%.cc=%.o)



#
# The default target, our editor.
#
default: kilua


#
# Build the editor.
#
kilua: $(OBJECTS)
	$(LINKER) ../$@ $(LFLAGS) $(OBJECTS) $(LDLIBS)

#
# Cleanup
#
clean:
	rm -f editor *.orig core *.o


#
# Indent our .cc + .h files.
#
.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 *.cc *.h
