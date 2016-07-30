
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
    }

    lua_pushstring(L, buffer->m_syntax.c_str());
    return 1;
}
