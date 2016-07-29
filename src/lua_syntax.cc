
#include <clocale>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include "editor.h"
#include "lua_primitives.h"



/**
 * Get/Set the syntax mode.
 */
int syntax_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    Buffer *buffer = e->current_buffer();

    if (lua_isstring(L, -1))
    {
        const char *mode = lua_tostring(L, -1);
        buffer->m_syntax = mode;
        buffer->m_colours.clear();
    }

    lua_pushstring(L, buffer->m_syntax.c_str());
    return 1;
}


/*
 * Usage: syntax_range( start, end, colour )
 */
int syntax_range_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    Buffer *buffer = e->current_buffer();

    int start  = lua_tonumber(L, -3);
    int end    = lua_tonumber(L, -2);
    int colour = lua_tonumber(L, -1);

    if (colour >= 9)
    {
        e->set_status(1, "args are bogus: %d-%d -> %d", start, end, colour);
        return 0;
    }

    FILE* fp = fopen("test.txt", "a");

    if (fp)
    {
        fprintf(fp, "%d-%d -> %d\n", start, end, colour);
        fclose(fp);
    }

    if (buffer->m_syntax.empty())
        return 0;


    /*
     * Set the colours - starting from the start
     */
    for (int i = start; i <= start + end; i++)
    {
        /*
         * Set the value
         */
        buffer->m_colours[i] =  colour ;
    }
}
