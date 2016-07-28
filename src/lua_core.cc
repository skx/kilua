
#include <clocale>
#include <cstdlib>
#include <string.h>
#include <malloc.h>

#include "editor.h"
#include "lua_primitives.h"
#include "util.h"

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

        if (ch == '\n')
        {
            e->set_status(0, "");

            char *out = Util::wide2ascii(input);
            lua_pushstring(L, out);
            delete(out);
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
        else if (len < sizeof(input))
        {
            input[len] = ch;
            len ++;
        }
    }

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

    if ((input = fopen(path, "r")) == NULL)
    {
        e->set_status(1, "Failed to open %s", path);
        return 0;
    }

    wchar_t c;

    while (1)
    {
        c = fgetwc(input);

        if (c == WEOF)
            break;

        e->insert(c);
    }

    fclose(input);

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

    buffer->set_dirty(false);
    return (0);
}


/**
 *  Delete a character.
 */
int delete_lua(lua_State *L)
{
    Editor *e      = Editor::instance();
    e->delete_char();

    Buffer *buffer = e->current_buffer();
    buffer->set_dirty(true);

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

    delete(wide);


    Buffer *buffer = e->current_buffer();
    buffer->set_dirty(true);

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
            delete(ascii);
            return 1;
        }
    }

}
