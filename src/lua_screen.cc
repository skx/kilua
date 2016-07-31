/* lua_screen.cc - Implementation of our screen-related Lua primitives.
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
    erow *row = nullptr;

    if (y < (int)buffer->rows.size())
        row = buffer->rows.at(y);

    if (row)
    {
        int len = row->chars->size();

        if (x < len)
        {
            res = row->chars->at(x);
        }
    }

    char *str = new char[7];
    sprintf(str, "%ls", res.c_str());
    lua_pushstring(L, str);
    delete []str;
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
