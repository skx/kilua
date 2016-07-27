
#include <clocale>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include "editor.h"
#include "lua_primitives.h"



/**
 * Move to the end of the file.
 */
int eof_lua(lua_State *L)
{
    (void)L;
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();
    buffer->cx = buffer->coloff = 0;
    buffer->cy = buffer->rowoff = 0;

    /*
     * Move down until we're at the end of file.
     */
    int max_row = buffer->rows.size();

    while (max_row)
    {
        e->move("down");
        max_row--;
    }

    eol_lua(L);

    return 0;
}


/**
 * Move to the end of the line.
 */
int eol_lua(lua_State *)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    /*
     * Length is enough to fit.
     */
    erow *row = buffer->rows.at(buffer->cy + buffer->rowoff);

    if (row->chars->size() < e->width())
    {
        buffer->coloff = 0;
        buffer->cx = row->chars->size() ;
    }
    else
    {
        buffer->cx = e->width() - 1;
        buffer->coloff = row->chars->size() - e->width() + 1;
    }
}


/**
 * Move the cursor in a given direction.
 */
int move_lua(lua_State *L)
{
    /*
     * Let us start out by working out where we are, the next row,
     * the previous row, etc.
     */
    Editor *e     = Editor::instance();
    const char *x = lua_tostring(L, -1);

    if (x)
        e->move(x);

    return (0);
}


/**
 * Move to the start of the buffer.
 */
int sof_lua(lua_State *L)
{
    (void)L;
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();
    buffer->cx = buffer->coloff = 0;
    buffer->cy = buffer->rowoff = 0;
    return 0;
}


/**
 * Move to the start of the line.
 */
int sol_lua(lua_State *)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();
    buffer->cx = 0;
    buffer->coloff = 0;
}
