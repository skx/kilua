Kilo
===

Kilo is a small text editor with around 1K lines of code (counted with cloc):

     -------------------------------------------------------------------
     Language        files          blank        comment           code
     -------------------------------------------------------------------
     C                   1            152            210           1138
     Lua                 2             47            119            153
     -------------------------------------------------------------------

A screencast is available here: https://asciinema.org/a/90r2i9bq8po03nazhqtsifksb

Usage: kilo `<filename>`

Keys:

    Ctrl-d: Insert date/time
    CTRL-l: Evaluate Lua
    CTRL-o: Open
    CTRL-s: Save
    CTRL-q: Quit
    CTRL-f: Find string in file (ESC to exit search, arrows to navigate)

Kilo does not depend on any library except for Lua5.2.

For graphics neither `ncurses` nor `curses` is used, instead it uses fairly
standard VT100 (and similar terminals) escape sequences. The project is in
alpha stage and was written in just a few hours taking code from my other two
projects, load81 and linenoise.

People are encouraged to use it as a starting point to write other editors
or command line interfaces that are more advanced than the usual REPL
style CLI.

Kilo was written by Salvatore Sanfilippo aka antirez and is released
under the BSD 2 clause license.


Lua-Fork
--------

This fork of `kilo` was hacked together by [Steve Kemp](https://steve.kemp.fi/)
to include a lua intepreter, and delegate as much as possible of the operation
to that lua instance.

* On startup `kilo.lua` is loaded from the current directory.
* Input is processed via the `on_key()` function defined in that file.
* We use a global keymap to link function-keys and lua functions.
* I've implemented trivial copy/paste support, but without the ability to select and mark "regions" this is ropy at best, and terrible at worst.
* There is a notion of a MARK.  A mark can be made by pressing `Ctrl-space`.
    * The mark will be shown with a white-background.
    * The region between the cursor and the mark may be cut via `Ctrl-w`,
      just like in Emacs.

The following primitives are exported to lua:

* at()
    * Return the character under the cursor.
* cut_selection()
    * Return the text between point and mark.
* delete()
    * Delete a single character (backwards).
* dirty()
    * Returns `true` if the buffer is dirty/modified, `false` otherwise.
* eol()
    * Move the cursor to the end of the current line.
* exit()
    * Terminate the editor.
* eval()
    * Prompt for lua code, and execute it.
* find()
    * Invoke the find-handler.
* kill()
    * Kill the current line, deleting it from display.
* key()
    * Read a single key from the keyboard (blocking).
    * Used to implement setting of marks.
* insert("string")
    * Inserts the given string at the current cursor position.  (Newlines work as expected.)
* mark()
   * get the position of the mark.
   * A return value of (-1,-1) means there is no mark set.
* mark(x,y)
   * set the position of the mark.
* open( [filename] )
    * Open the named file for reading.
    * If the filename is not given, prompt for one.
* page_up()
    * Scroll the screen up one page, if possible.
* page_down()
    * Scroll the screen down one page, if possible.
* point()
    * Return the current position of the cursor.
* point(x,y)
    * Move the cursor to the given position.
    * Used to implement marks.
* prompt( "prompt" )
    * Prompt the user for input, and return it to lua.
* save()
    * Save the file we're operating upon.
* selection()
    * Return the text between point and mark.
* set_syntax_comments()
    * Setup comment-handling for syntax-highlighting.
* set_syntax_keywords()
    * Setup keyword-handling for syntax-highlighting.
* sol()
    * Move the cursor to the start of the current line.

In addition to those functions there are also the movement-related primitives: up(), down(), left(), right().


Callbacks
---------

There is room for lots of functionality to be delegated to callbacks,
implemented in lua, but right now there are just three:

* `on_key()`
    * Called to process a single key input.
* `on_loaded(filename)`
    * Called when a file is loaded.
    * This sets up syntax highlighting in our default implementation for C and Lua files.
* `on_saved(filename)`
    * Called __after__ a file is saved.
    * Can be used to make files executable, etc.


Copy & Paste
------------

We've added a notion of a `mark`.  A mark is set by pressing `Ctrl+space`,
and at any time you can cut the region between the cursor and the mark by
pressing `Ctrl-w`.

You can also cut the current line via `Ctrl-y`.

In both cases you can yank the selection back with `Ctrl-u`.


Syntax Highlighting
-------------------

Syntax highlighting is defined in lua, and configured by calling:

    -- Setup "keywords"
    set_syntax_keywords({ "void|", "int|", "while", "true" } )

    -- Setup "comments"
    set_syntax_comments( "//", "/*", "*/" )

    -- Enable highlighting of numbers/strings
    syntax_highlight_numbers( 0 | 1 )
    syntax_highlight_strings( 0 | 1 )


The `on_loaded()` function has an example covering both C/C++ and Lua.

If you wish to change the syntax-highlighting, once a file is loaded,
you can do that by entering `Ctrl-l` to open the Lua-prompt then
typing (for example):

    set_syntax( "pl" )

Currently we include syntax for Perl, Lua, C, C++, and shell.


The Future?
-----------

Future plans?

* The save() primitive should allow:  save( "new/file/name" )


Steve
--
https://steve.kemp.fi/
