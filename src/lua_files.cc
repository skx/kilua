/* lua_files.cc - Implementation of our filesystem-related lua primitives.
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

#include <string>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

#include "lua_primitives.h"


/**
 * Get the files in the given directory.
 */
int directory_entries_lua(lua_State *L)
{
    const char *str = lua_tostring(L, 1);
    std::vector < std::string > result;

    /*
     * Get the entries in the given path.
     */
    DIR *dp = opendir(str);

    if (dp != NULL)
    {
        result.push_back(str);
        dirent *de;

        while ((de = readdir(dp)) != NULL)
        {
            std::string r = str;
            r += "/";
            r += de->d_name;
            result.push_back(r);
        }
    }

    closedir(dp);

    /*
     * Sort them, to be nice.
     */
    std::sort(result.begin(), result.end());


    /*
     * If we got results then we're good, otherwise return nil
     */
    if (result.empty())
    {
        lua_pushnil(L);
        return 1;
    }


    /*
     * Now put those entries into a table.
     */
    lua_newtable(L);
    int i = 1;

    for (auto it = result.begin(); it != result.end(); ++it)
    {
        std::string value = (*it);

        lua_pushinteger(L, i);
        lua_pushstring(L, value.c_str());

        lua_settable(L, -3);

        i += 1;
    }

    return 1;
}



/**
 * Does the named file exist?
 */
int exists_lua(lua_State *L)
{
    const char *str = lua_tostring(L, 1);

    if (str == NULL)
    {
        lua_pushnil(L);
        return 1;
    }

    struct stat sb;

    if ((stat(str, &sb) == 0))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

