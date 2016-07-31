/* main.cc - Main application entry-point.
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

#include <locale.h>
#include <getopt.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#include "config.h"
#include "editor.h"



/**
 * Setup the curses environment, along with the colours.
 */
void setup()
{
    char e[] = "ESCDELAY=0";
    putenv(e);
    setlocale(LC_ALL, "");
    initscr();
    start_color();

    if (has_colors())
    {
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_CYAN,    COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
        /* reverse */
        init_pair(8,  COLOR_WHITE, COLOR_RED);
        init_pair(9,  COLOR_WHITE, COLOR_GREEN);
        init_pair(10, COLOR_WHITE, COLOR_YELLOW);
        init_pair(11, COLOR_WHITE, COLOR_BLUE);
        init_pair(12, COLOR_WHITE, COLOR_MAGENTA);
        init_pair(13, COLOR_WHITE, COLOR_CYAN);
    }
    else
    {
        endwin();
        fprintf(stderr, "No colours!\n");
        exit(1);
    }

    raw();
    keypad(stdscr, TRUE);
    noecho();
    timeout(750);
}


/**
 * Exit from curses
 */
void teardown()
{
    endwin();
}


/**
 * Setup the console, and start the editor.
 */
int main(int argc, char *argv[])
{

    /*
     * Setup curses.
     */
    setup();

    /*
     * Create a new editor.
     */
    Editor *e = Editor::instance();

    /*
     * Parse command-line options.
     */
    while (1)
    {
        static struct option long_options[] =
        {
            {"config", required_argument, 0, 'c'},
            {"dump-config", no_argument, 0, 'd'},
            {"syntax-path", required_argument, 0, 's'},
            {"version", no_argument, 0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        char c = getopt_long(argc, argv, "c:s:vd", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 'c':
            e->load_lua(optarg);
            break;

        case 'd':
            endwin();
            printf("%s\n", kilua_lua);
            exit(0);
            break;

        case 's':
            e->set_syntax_path(optarg);
            break;

        case 'v':
            endwin();
            fprintf(stderr, "kilua v£π\n");

            exit(0);
            break;
        }
    }




    /*
     * Load our default configuration file ~/.kilua/init.lua
     */
    int loaded = 0;

    char init_buf[1024] = {'\0'};
    snprintf(init_buf, sizeof(init_buf) - 1, "%s%s",
             getenv("HOME"), "/.kilua/init.lua");
    loaded += e->load_lua(init_buf);

    /*
     * Load our default configuration file ~/.kilua/$hostname.lua
     */
    char *hostname = e->hostname();
    snprintf(init_buf, sizeof(init_buf) - 1, "%s/.kilua/%s.lua",
             getenv("HOME"), hostname);
    loaded += e->load_lua(init_buf);
    free(hostname);

    /*
     * If we loaded nothing use the default.
     */
    if (loaded == 0)
        e->eval_lua((const char *)kilua_lua);

    /*
     * Filenames we'll load in our editor session.
     */
    std::vector<std::string> files;

    /*
     * For each file on the cmmand line - load it.
     */
    if (argc - optind)
    {
        for (int i = 0; i < (argc - optind); i++)
            files.push_back(argv[optind + i]);
    }

    /*
     * Load the files.
     */
    e->load_files(files);

    /*
     * Initial render.
     */
    e->draw_screen();

    /*
     * Run main-loop - this never terminates.
     */
    e->main_loop();

    /*
     * But we pretend it does, and cleanup.
     */
    teardown();
    return 0;
}
