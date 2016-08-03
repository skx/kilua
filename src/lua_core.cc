/* lua_core.cc - Implementation of our core lua primitives.
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
#include <regex.h>

#include "editor.h"
#include "lua_primitives.h"
#include "util.h"



/**
 *  Delete a character.
 */
int delete_lua(lua_State *L)
{
    (void)L;
    Editor *e      = Editor::instance();
    e->delete_char();

    /*
     * The buffer is now dirty and needs to be re-rendered.
     */
    Buffer *buffer = e->current_buffer();
    buffer->set_dirty(true);
    buffer->touch();

    return 0;
}


/**
 * Is the current buffer dirty?
 */
int dirty_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    if (buffer->dirty())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}


/**
 * Exit the application.
 */
int exit_lua(lua_State *L)
{
    (void)L;
    endwin();
    exit(0);
    return 0;
}


/**
 * Insert a character-string.
 */
int insert_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return 0;

    /*
     * Convert the input to wide characters.
     */
    wchar_t *wide = Util::ascii2wide(str);
    int size = wcslen(wide);

    for (int i = 0; i < size ; i++)
        e->insert(wide[i]);

    delete []wide;

    /*
     * The buffer is now dirty and needs to be re-rendered.
     */
    Buffer *buffer = e->current_buffer();
    buffer->set_dirty(true);
    buffer->touch();
    return 0;
}


/**
 * Read a single (wide) key.
 */
int key_lua(lua_State *L)
{
    Editor *e = Editor::instance();

    while (1)
    {
        e->draw_screen();
        unsigned int ch;

        int res = get_wch(&ch);

        /*
         * Chances are this was a timeout.
         *
         * So invoke the handler and redraw.
         */
        if (res == ERR)
        {
        }
        else
        {
            /*
             * Convert the character to a string.
             */
            char *ascii = Util::wchar2ascii(ch);
            lua_pushstring(L, ascii);
            delete []ascii;
            return 1;
        }
    }

}


/**
 * Given a table of strings let the user select one, via a menu.
 */
int menu_lua(lua_State *L)
{
    Editor *e = Editor::instance();

    if (!lua_istable(L, 1))
    {
        e->set_status(1, "Table expected!");
        return 0;
    }

    /*
     * Build up the list of choices the user has submitted.
     */
    std::vector < std::string > choices;

    lua_pushnil(L);

    while (lua_next(L, -2))
    {
        const char *entry = lua_tostring(L, -1);
        choices.push_back(entry);
        lua_pop(L, 1);
    }

    /*
     * Now the user will choose an entry, interactively.
     */
    int ret = e->menu(choices);
    lua_pushinteger(L, ret);
    return 1;
}


/*
 * Get/Set the point.
 */
int point_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    if (lua_isnumber(L, -2) && lua_isnumber(L, -1))
    {
        int y = lua_tonumber(L, -1) - 1;
        int x = lua_tonumber(L, -2) - 1;

        e->warp(x, y);
    }

    lua_pushnumber(L,  buffer->cx + buffer->coloff);
    lua_pushnumber(L,  buffer->cy + buffer->rowoff);
    return 2;
}


/*
 * Prompt for input in the status-area.
 */
int prompt_lua(lua_State *L)
{
    /*
     * Get the prompt
     */
    const char *prompt = lua_tostring(L, -1);

    /*
     * Get the editor.
     */
    Editor *e = Editor::instance();

    /*
     * Input buffer, and current offset.
     */
    wchar_t input[200];
    memset(input, '\0', sizeof(input));
    int len = 0;

    /*
     * combined, what we show to the status.
     */
    while (1)
    {
        /*
         * Build up the combined string of "prompt" + current input
         */
        int c_len = wcslen(input) + strlen(prompt);
        char *c_txt = (char *)malloc(c_len + 1);

        sprintf(c_txt, "%s%ls", prompt, input);

        /*
         * Display the new prompt.
         */
        e->set_status(0, c_txt);
        e->draw_screen();
        free(c_txt);

        /*
         * cap at last-x if the input is too long.
         */
        int y = len + strlen(prompt);

        if (y > e->width())
        {
            y = e->width() - 1;
        }

        /*
         * Show the cursor at the right place.
         */
        move(e->height() + 1, y);

        /*
         * poll for input.
         */
        unsigned int ch;

        int res = get_wch(&ch);

        if (res == ERR)
            continue;

        if (ch == '\t')
        {
            /*
             * Pass the current text to the callback.
             */
            char *current = Util::wide2ascii(input);

            /*
             * Call the handler.
             */
            char *completed = NULL;

            e->call_lua("on_complete", "s>s", current, &completed);

            if ( completed != NULL )
            {
                wchar_t *tmp = Util::ascii2wide(completed);
                wcscpy(input,  tmp);
                len = wcslen(tmp);
                delete[]tmp;
            }

            delete []current;

        }
        else if (ch == '\n')
        {
            e->set_status(0, "");

            char *out = Util::wide2ascii(input);
            lua_pushstring(L, out);
            delete []out;
            return 1;
        }
        else if (ch == KEY_BACKSPACE)
        {
            len --;

            if (len < 0)
                len = 0;

            input[len] = '\0';

        }
        else if (ch == 27)
        {
            e->set_status(0, "Cancelled");
            return 0;
        }
        else if (len < (int)sizeof(input))
        {
            input[len] = ch;
            len ++;
        }
    }

    return 0;
}


/*
 * Open a file in Lua.
 */
int open_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();
    buffer->empty_buffer();

    /*
     * Name of the buffer.
     */
    const char *path  = buffer->get_name();

    /*
     * Did we get a different name?
     */
    const char *new_name = lua_tostring(L, -1);

    if (new_name != NULL)
    {
        buffer->set_name(new_name);
        path = new_name;
    }

    FILE *input;

    if ((input = fopen(path, "r")) != NULL)
    {
        wchar_t c;

        while (1)
        {
            c = fgetwc(input);

            if (c == (wchar_t)WEOF)
                break;

            e->insert(c);
        }

        fclose(input);
    }
    else
    {
        e->set_status(1, "Failed to open %s", path);
    }

    /*
     * Call the handler.
     */
    e->call_lua("on_loaded", "s>", path);

    /*
     * Move to start of file.
     */
    sof_lua(NULL);
    buffer->set_dirty(false);
    return (0);
}


/**
 * Save the current file.
 */
int save_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();


    /*
     * Name of the buffer.
     */
    const char *path  = buffer->get_name();

    /*
     * Did we get a different name?
     */
    const char *new_name = lua_tostring(L, -1);

    if (new_name != NULL)
    {
        buffer->set_name(new_name);
        path = new_name;
    }

    FILE *handle;

    if ((handle = fopen(path, "w")) == NULL)
    {
        e->set_status(1, "Failed to open %s for writing", path);
        return 0;
    }

    /*
     * For each row.
     */
    int rows = buffer->rows.size();

    for (int y = 0; y < rows; y++)
    {
        /*
         * For each character
         */
        int chars = buffer->rows.at(y)->chars->size();

        for (int x = 0; x < chars; x++)
        {
            std::wstring chr = buffer->rows.at(y)->chars->at(x);
            fprintf(handle, "%ls", chr.c_str());
        }

        fprintf(handle, "\n");
    }

    fclose(handle);

    /*
     * Call the handler.
     */
    e->call_lua("on_saved", "s>", path);

    buffer->set_dirty(false);
    return (0);
}


/*
 * Search (fowards) for a regexp
 */
int search_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    Buffer *buffer = e->current_buffer();

    /*
     * Get the search pattern.
     */
    const char *pattern = lua_tostring(L, -1);

    if (pattern == NULL)
    {
        e->set_status(1, "There was no regular expression supplied!");
        return 0;
    }

    /*
     * Compile the pattern as a regular expression.
     */
    regex_t regex;

    if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE) != 0)
    {
        e->set_status(1, "Failed to compile %s as a regular expression!", pattern);
        return 0;
    };

    /*
     * Count the number of rows we have in the buffer.
     */
    int rows = buffer->rows.size();

    /*
     * The first line is special.
     */
    bool first = true;

    /*
     * Searching fowwards ..
     */
    int step = 1;

    /*
     * The starting offset into the buffer.
     */
    int offset = buffer->cy + buffer->rowoff;

    /*
     * For each row in the buffer
     *
     * NOTE: We deliberately search for ONE TOO MANY rows here.
     *
     * THis means if the point is a "skx:[POINT]" a search for
     * "^skx" will match.
     *
     * Since we start searching the first line (the line with the point)
     * at the current cursor position we'd otherwise fail to find this
     * match.
     *
     */
    for (int i = 0; i <= rows ; i ++)
    {
        /*
         * Ensure we wrap around the buffer, rather than
         * walking off the end of the list of rows.
         */
        if (offset >= rows)
            offset = 0;

        if (offset < 0)
            offset = rows - 1;

        /*
         * Get the current row.
         */
        erow *row = buffer->rows.at(offset);

        /*
         * Now we need to search for the given text
         * in the row.
         *
         * If we're in the first row we search from the
         * current X-position, otherwise we search from
         * the start of the line (ie. offset zero).
         */
        int x = 0;

        if (first)
            x = buffer->cx + buffer->coloff;


        /*
         * Get the text in the row.
         */
        std::string text;

        while (x < (int)row->chars->size())
        {
            std::wstring character = row->chars->at(x);
            char *c_txt = (char *)malloc(wcslen(character.c_str()) + 1);
            sprintf(c_txt, "%ls", character.c_str());

            text += c_txt;
            free(c_txt);
            x++;
        }

        /*
         * regexps match on txt;
         */
        regmatch_t result[1];
        int res = regexec(&regex, text.c_str(), 1, result, 0);

        /*
         * Did we match?
         */
        if (res == 0)
        {
            /*
             * The offset of the match.
             */
            int pos = result[0].rm_so;

            if (first)
                pos += (buffer->cx + buffer->coloff);

            /*
             * Move to the right row.
             */
            buffer->cy     = 0;
            buffer->rowoff = offset;

            /* move to start of line. */
            sol_lua(L);

            /* move right enough times to move to the start of the match. */
            while (pos > 0)
            {
                e->move("right");
                pos--;
            }

            return 0;
        }


        /*
         * Now we're searching the next line.
         */
        offset += step;
        first = false;


    }

    e->set_status(1, "No match found!");
    return 0;
}


/*
 * Update the status-text
 */
int status_lua(lua_State *L)
{
    const char *x = lua_tostring(L, -1);
    Editor *e = Editor::instance();
    e->set_status(1, x);
    return (0);
}
