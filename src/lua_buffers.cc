
#include <clocale>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include "editor.h"
#include "lua_primitives.h"


/*
 * Get/Set current buffer.
 */
int buffer_lua(lua_State *L)
{
    Editor *e = Editor::instance();


    if (lua_isnumber(L, -1))
    {
        e->set_current_buffer(lua_tonumber(L, -1));
    }

    int cur = e->get_current_buffer();
    lua_pushnumber(L, cur);
    return 1;
}

/*
 * Count buffers
 */
int buffers_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    int mx    = e->count_buffers();
    lua_pushnumber(L, mx);
    return 1;
}

/*
 * Create a new buffer.
 */
int create_buffer_lua(lua_State *L)
{
    /*
     * The name of the buffer.
     */
    const char *name = lua_tostring(L, -1);

    Editor *e = Editor::instance();
    e->new_buffer(name);
    return 0;
}

/*
 * Get/Set the name of the buffer.
 */
int buffer_name_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    Buffer *buffer = e->current_buffer();

    const char *name = lua_tostring(L, -1);

    if (name)
    {
        buffer->set_name( name );
    }

    lua_pushstring(L, buffer->get_name());
    return 1;
}

/**
 * Kill the current buffer.
 */
int kill_buffer_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    e->kill_current_buffer();
}

/**
 * Select a buffer, by name.
 */
int select_buffer_lua(lua_State *L)
{
    /*
     * Get the name we're to select.
     */
   const char *name = lua_tostring(L, -1);
   if ( name ==  NULL ){
        lua_pushboolean(L, 0);
        return 1;
    }


    /*
     * Find the buffer.
     */
    Editor *e = Editor::instance();
    int off   = e->buffer_by_name( name );

    if ( off != -1 )
    {
        e->set_current_buffer(off);
        lua_pushboolean(L, 1 );
    }
    else
    {
        lua_pushboolean(L, 0);
        return 1;
    }

}
