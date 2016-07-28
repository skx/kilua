
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
     * Default return value.
     */
    std::wstring res;

    /*
     * Get the row.
     */
    erow *row;

    if (y < buffer->rows.size())
        row = buffer->rows.at(y);

    if (row)
    {
        int len = row->chars->size();

        if (x < len)
        {
            res = row->chars->at(x);
        }
    }

    char *str = new char[2];
    sprintf(str, "%ls", res.c_str());
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
