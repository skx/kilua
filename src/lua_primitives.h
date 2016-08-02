/* lua_primitives.h - Definitions of the Lua primitives.
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
extern int insert_lua(lua_State *L);
extern int key_lua(lua_State *L);
extern int menu_lua(lua_State *L);
extern int open_lua(lua_State *L);
extern int point_lua(lua_State *L);
extern int prompt_lua(lua_State *L);
extern int save_lua(lua_State *L);
extern int search_lua(lua_State *L);
extern int status_lua(lua_State *L);

/*
 * Files.
 */
extern int directory_entries_lua(lua_State *L);
extern int exists_lua(lua_State *L);

/*
 * Screen.
 */
extern int at_lua(lua_State *L);
extern int height_lua(lua_State *L);
extern int width_lua(lua_State *L);

/*
 * Syntax
 */
extern int syntax_lua(lua_State *L);

/*
 * Buffers
 */
extern int buffer_lua(lua_State *L);
extern int buffer_name_lua(lua_State *L);
extern int buffers_lua(lua_State *L);
extern int create_buffer_lua(lua_State *L);
extern int kill_buffer_lua(lua_State *L);
