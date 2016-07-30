/* lua_movement.cc - Implementation of movement-related lua primitives.
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
int eol_lua(lua_State *L)
{
    (void)L;
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    /*
     * Length is enough to fit.
     */
    erow *row = buffer->rows.at(buffer->cy + buffer->rowoff);

    if ((int)row->chars->size() < e->width())
    {
        buffer->coloff = 0;
        buffer->cx = row->chars->size() ;
    }
    else
    {
        buffer->cx = e->width() - 1;
        buffer->coloff = row->chars->size() - e->width() + 1;
    }

    return 0;
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
int sol_lua(lua_State *L)
{
    (void)L;
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();
    buffer->cx = 0;
    buffer->coloff = 0;
    return 0;
}
