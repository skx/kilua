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
 * Count the buffers.
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
 * Select a buffer, interactively.
 */
int choose_buffer_lua(lua_State *L)
{
    (void)L;
    Editor *e = Editor::instance();
    int selected = e->get_current_buffer();
    int original = e->get_current_buffer();
    int max      = e->count_buffers();

    /*
     * Hide the cursor
     */
    curs_set(0);
    ::clear();

    while (true)
    {
        for (int i = 0; i < max ; i++)
        {
            if (selected == i)
                attron(A_STANDOUT);

            e->set_current_buffer(i);
            Buffer *b = e->current_buffer();

            std::string tmp;
            tmp = std::to_string(i + 1);
            tmp += " ";
            tmp += b->get_name();
            tmp += " ";

            if (b->dirty())
                tmp += " <modified>";

            while ((int)tmp.size() < e->width())
                tmp += " ";

            mvwaddstr(stdscr, i, 0, tmp.c_str());

            if (selected == i)
                attroff(A_STANDOUT);
        }

        /*
         * poll for input.
         */
        unsigned int ch;

        int res = get_wch(&ch);

        if (res == ERR)
            continue;

        if (ch == '\n')
        {
            curs_set(1);
            e->set_current_buffer(selected);
            return 0;
        }

        if (ch == 27)
        {
            curs_set(1);
            e->set_current_buffer(original);
            return 0;
        }

        if (ch == KEY_UP)
        {
            selected -= 1;

            if (selected < 0)
                selected = max - 1;
        }

        if (ch == KEY_DOWN)
        {
            selected += 1;

            if (selected >= max)
                selected = 0;
        }

    }

    return 0;
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
