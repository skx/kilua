/* editor.h - Our editor class.
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

#pragma once

#include <string.h>
#include <ncurses.h>


#include "data.h"
#include "lua_primitives.h"
#include "singleton.h"


/**
 * The editor instance, which is a singleton.
 *
 * An editor instance contains a series of buffer-objects, as well
 * as a global Lua handle.
 *
 */
class Editor : public Singleton<Editor>
{
public:
    /**
     * Constructor.
     */
    Editor();

    /**
     * Destructor.
     */
    ~Editor();

public:
    /**
     * Called when the program is launched.
     *
     * If we've been given an array of files:
     *   * For each file, create a buffer and open it.
     * Otherwise create a single buffer for working with.
     */
    void load_files(std::vector<std::string> files);

    /**
     * The main loop.
     *
     * Read keys constantly, pass them to Lua, then refresh the display.
     *
     */
    void main_loop();

    /**
     * Insert the given character into the current position in the
     * buffer.
     */
    void insert(wchar_t c);

    /**
     * Delete one character, backwards, from the current position.
     */
    void delete_char();

    /**
     * Get the current buffer.
     *
     * Public so that the `move` primitive, etc, can access it.
     */
    Buffer *current_buffer();

    /**
     * Lookup the index of the buffer with the specified name.
     *
     * Returns -1 on failure.
     */
    int buffer_by_name(const char *name);

    /**
     * Get the status-text.
     */
    char * get_status();

    /**
     * Update the status-text.
     */
    void set_status(int log, const char *fmt, ...);


    /**
     * Update the syntax of the buffer.
     */
    void update_syntax();

    /**
     * Clear the screen and redraw it.
     */
    void draw_screen();

    /**
     * Get the height of our editing area.
     */
    int height();

    /**
     * Get the width of our editing area.
     */
    int width();

    /**
     * Call lua
     */
    void call_lua(const char *func, const char *sig, ...);

    /**
     * Load a Lua file, if it exists, and execute it.
     */
    int load_lua(const char *filename);

    /**
     * Move the cursor to the given position, if possible.
     */
    void warp(int x, int y);

    /**
     * Move the cursor in the given direction.
     */
    void move(const char *direction);

    /**
     * Buffer handling.
     */
    int  get_current_buffer();
    int  count_buffers();
    void set_current_buffer(int off);
    void new_buffer(const char *name = "unnamed");
    void kill_current_buffer();

    /**
     * Get the hostname we're running on.
     */
    char *hostname();

private:

    /**
     * Lookup the long-name for the given key.
     */
    const char *lookup_key(unsigned int c);

    /**
     * Our state.
     */
    editorState *m_state;

    /**
     * Our lua object.
     */
    lua_State * m_lua;
};
