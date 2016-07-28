#pragma once

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "data.h"


/*
 * Movement.
 */
extern int eof_lua(lua_State *L);
extern int eol_lua(lua_State *L);
extern int move_lua(lua_State *L);
extern int sof_lua(lua_State *L);
extern int sol_lua(lua_State *L);


/*
 * Core
 */
extern int delete_lua(lua_State *L);
extern int dirty_lua(lua_State *L);
extern int exit_lua(lua_State *L);
extern int height_lua(lua_State *L);
extern int insert_lua(lua_State *L);
extern int key_lua(lua_State *L);
extern int open_lua(lua_State *L);
extern int point_lua(lua_State *L);
extern int prompt_lua(lua_State *L);
extern int save_lua(lua_State *L);

/*
 * Screen.
 */
extern int at_lua(lua_State *L);
extern int status_lua(lua_State *L);
extern int width_lua(lua_State *L);


/*
 * Buffers
 */
extern int buffer_lua(lua_State *L);
extern int buffers_lua(lua_State *L);
extern int choose_buffer_lua(lua_State *L);
extern int create_buffer_lua(lua_State *L);
extern int buffer_name_lua(lua_State *L);
extern int kill_buffer_lua(lua_State *L);
extern int select_buffer_lua(lua_State *L);
