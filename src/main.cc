#include <locale.h>
#include <getopt.h>
#include "editor.h"

void setup()
{
    char e[] = "ESCDELAY=0";
    putenv(e);
    setlocale(LC_ALL, "");
    initscr();
    start_color();

    if (has_colors())
    {
        init_pair(1,  COLOR_RED,     COLOR_BLACK);
        init_pair(2,  COLOR_GREEN,   COLOR_BLACK);
        init_pair(3,  COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4,  COLOR_BLUE,    COLOR_BLACK);
        init_pair(5,  COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6,  COLOR_CYAN,    COLOR_BLACK);
        init_pair(7,  COLOR_BLUE,    COLOR_BLACK);
        init_pair(8,  COLOR_WHITE,   COLOR_BLACK);
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

void teardown()
{
    endwin();
}

/*
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
            {"version", no_argument, 0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        char c = getopt_long(argc, argv, "c:v", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 'c':
            e->load_lua(optarg);
            break;

        case 'v':
            endwin();
            fprintf(stderr, "kilua v£π\n");

            exit(0);
            break;
        }
    }


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
