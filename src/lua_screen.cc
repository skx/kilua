
#include <clocale>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include "editor.h"
#include "lua_primitives.h"



/**
 * Get the character at the cursor position.
 */
int at_lua(lua_State *L)
{
    /*
     * Get the buffer.
     */
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    /*
     * Get the cursor position.
     */
    int x = buffer->cx + buffer->coloff;
    int y = buffer->cy + buffer->rowoff ;

    /*
     * Read the character
     */
    wchar_t c = (mvinch(y, x) & A_CHARTEXT);

    /*
     * Convert to ASCII
     */
    std::wstring tmp;
    tmp += c;

    char *str = new char[255];
    sprintf(str, "%ls", tmp.c_str());

    lua_pushstring(L, str);
    delete(str);
    return 1;
}


/**
 * Get the height of the drawing-area - minus the two line footer.
 */
int height_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    lua_pushnumber(L, e->height());
    return 1;
}


/**
 * Get the width of the screen.
 */
int width_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    lua_pushnumber(L, e->width());
    return 1;
}
