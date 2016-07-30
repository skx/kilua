
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

/**
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

/**
 * Kill the current buffer.
 */
int kill_buffer_lua(lua_State *L)
{
    (void)L;
    Editor *e = Editor::instance();
    e->kill_current_buffer();
    return 0;
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

    if (name ==  NULL)
    {
        lua_pushboolean(L, 0);
        return 1;
    }


    /*
     * Find the buffer.
     */
    Editor *e = Editor::instance();
    int off   = e->buffer_by_name(name);

    if (off != -1)
    {
        e->set_current_buffer(off);
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
}
