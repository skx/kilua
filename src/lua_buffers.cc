/* lua_buffers.cc - Implementation of our buffer-related lua primitives.
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (C) 2016 Steve Kemp https://steve.kemp.fi/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <clocale>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include "editor.h"
#include "lua_primitives.h"


/*
 * Get/Set per-buffer data.
 */
int buffer_data_lua(lua_State *L)
{
    /*
     * Get the buffer.
     */
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    /*
     * One argument is a lookup.
     */
    if (lua_gettop(L) == 1)
    {
        const char *key = lua_tostring(L, -1);
        lua_pushstring(L, buffer->get_data(key).c_str());
        return 1;
    }

    /*
     * Two arguments means to set.
     */
    if (lua_gettop(L) == 2)
    {
        const char *key = lua_tostring(L, -2);
        const char *val = lua_tostring(L, -1);
        buffer->set_data(key, val);
        return 0;
    }

    /*
     * Anything else is an error
     */
    e->set_status(1, "Invalid argument count!");
    return 0;


}

/*
 * Get/Set current buffer.
 */
int buffer_lua(lua_State *L)
{
    Editor *e = Editor::instance();

    /*
     * If called with a number, then select the given buffer.
     */
    if (lua_isnumber(L, -1))
    {
        int off = lua_tonumber(L, -1);
        e->set_current_buffer(off);
    }

    /*
     * If called with a string then select the buffer
     * by name, and return the offset.
     */
    if (lua_isstring(L, -1))
    {
        const char *name = lua_tostring(L, -1);
        int off          = e->buffer_by_name(name);

        if (off != -1)
            e->set_current_buffer(off);

        lua_pushnumber(L, off);
        return 1;
    }


    /*
     * Return the number of the current buffer.
     */
    int cur = e->get_current_buffer();
    lua_pushnumber(L, cur);
    return 1;
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
        buffer->set_name(name);
    }

    lua_pushstring(L, buffer->get_name());
    return 1;
}


/*
 * Return a table of all known buffers.
 */
int buffers_lua(lua_State *L)
{
    Editor *e = Editor::instance();
    std::vector<Buffer *>buffers = e->get_buffers();

    lua_createtable(L, buffers.size(), 0);


    for (int i = 0; i < (int)buffers.size(); i++)
    {
        Buffer *b = buffers.at(i);
        lua_pushstring(L, b->get_name());
        lua_rawseti(L, -2, i + 1);
    }

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
 * Kill the current buffer.
 */
int kill_buffer_lua(lua_State *L)
{
    (void)L;
    Editor *e = Editor::instance();
    e->kill_current_buffer();
    return 0;
}
