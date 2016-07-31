/* intro.h - Holder for our welcome-message.
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

#include <vector>
#include <string>

/*
 * This flag is set the first time a key is successfully pressed.
 *
 * It means that our buffer is no longer empty.
 */
extern bool one_key_pressed;


/*
 * This array contains text that is drawn when:
 *
 * * The default-buffer is empty.
 *
 * * There has not been a single key-press.
 */
extern std::vector<std::string> intro;


/*
 * Populate the default message.
 *
 * This message cannot be larger than the display-size of the
 * editor - since there is no support for scrolling it.
 */
void init_intro()
{
    if (intro.size() > 0)
        return;

    /*
     * Show the banner.
     */
    intro.push_back("kilua v" KILUA_VERSION);
    intro.push_back("----------");
    intro.push_back(" ");
    intro.push_back("Kilua was put together by Steve Kemp <https://steve.kemp.fi/>,");
    intro.push_back("based upon the prototype editor Antirez wrote and introduced");
    intro.push_back("upon his blog <http://antirez.com/news/108>.");
    intro.push_back(" ");
    intro.push_back(" ");
    intro.push_back("Major keybindings");
    intro.push_back("-----------------");
    intro.push_back(" ");
    intro.push_back(" Exit                 | Ctrl-x Ctrl-c");
    intro.push_back(" Create a new buffer  | Ctrl-x c");
    intro.push_back(" Switch buffers       | Ctrl-x b");
    intro.push_back(" Open a file          | Ctrl-x Ctrl-f");
    intro.push_back(" Save a file          | Ctrl-x Ctrl-s");
    intro.push_back(" Execute Lua code     | M-x");
    intro.push_back(" ");
    intro.push_back(" ");
    intro.push_back("For further details please see the project page on github:");
    intro.push_back(" ");
    intro.push_back(" https://github.com/skx/kilua");
    intro.push_back(" ");

}
