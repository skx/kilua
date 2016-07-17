[![Build Status](https://travis-ci.org/skx/kilo.png)](https://travis-ci.org/skx/kilo)

# Kilo

Kilo is a minimal text editor written in around 2K lines of code, as
counted with `cloc`:

     ----------------------------------------------------------------------
     Language            files          blank        comment           code
     ----------------------------------------------------------------------
     C                       1            419            309           1668
     Lua                     1             88            163            193
     ----------------------------------------------------------------------

You can launch the editor via :

    $ kilo /path/to/file

Once launched the arrow keys will move you around, and the main keybindings
to learn are:

    Ctrl-d: Insert date/time
    CTRL-l: Read lua from the user, and evaluate it.
    CTRL-o: Open a new file.
    CTRL-s: Save the current file.
    CTRL-q: Quit
    CTRL-f: Find string in file (ESC to exit search, arrows to navigate)


There's a screencast showing the differences between this fork and
the original `kilo` available here:

* https://asciinema.org/a/2tkjh1gog1y3e5jmpdk9a663k


## License

Kilo was written by Salvatore Sanfilippo, aka antirez, and is released
under the BSD 2 clause license.


## Lua Support

This fork of `kilo` was hacked together by [Steve Kemp](https://steve.kemp.fi/)
to include a lua intepreter, and delegate as much as possible of the operation
to that lua instance.

* On startup our initialization files are read:
    * `~/.kilo.lua`.
    * `./kilo.lua`.
    * If zero startup files are loaded this is a fatal error.
* Input is processed via the `on_key()` function defined in that file.
     * We use a global keymap to bind control-keys to lua functions.
* There is a notion of a MARK.  A mark can be made by pressing `Ctrl-space`.
    * The region between mark and cursor is known as the "selection", and will be shown with a white-background.

The following primitives are exported to lua:

* at()
    * Return the single character under the cursor.
* cut_selection()
    * Remove the text between curser and mark.
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
    * Inserts the given string at the current cursor position.
    * (Newlines work as expected.)
* mark()
   * get the position of the mark.
   * A return value of (-1,-1) means there is no mark set.
* mark(x,y)
   * Set the position of the mark - set `-1,-1` to remove the mark.
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
    * This is used to return to named marks.
* prompt( "prompt" )
    * Prompt the user for input, and return it to lua.
* save([filename])
    * Save the file we're operating upon.
    * If you give a new name that will be used thereafter.
* selection()
    * Return the text between cursor and mark.
* set_syntax_comments()
    * Setup comment-handling for syntax-highlighting.
* set_syntax_keywords()
    * Setup keyword-handling for syntax-highlighting.
* sol()
    * Move the cursor to the start of the current line.

In addition to those functions there are also the obvious movement-related primitives: `up()`, `down()`, `left()`, and `right()`.


## Callbacks

There is room for lots more functionality to be delegated to callbacks,
but right now there are just three:

* `on_key()`
    * Called to process a single key input.
* `on_loaded(filename)`
    * Called when a file is loaded.
    * This sets up syntax highlighting in our default implementation for C and Lua files.
* `on_saved(filename)`
    * Called __after__ a file is saved.
    * Can be used to make files executable, etc.


## Copy & Paste

We've added a notion of a `mark`.  A mark is set by pressing `Ctrl+space`,
and at any time you can cut the region between the cursor and the mark by
pressing `Ctrl-w`.

You can also cut the current line via `Ctrl-y`.

In both cases you can yank the selection back with `Ctrl-u`.


## Syntax Highlighting

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


## Undo Support

The basic actions of inserting characters, deleting characters, and
moving the cursor can be undo via the lua `undo` function.  (Which is
bound to `Ctrl-z` by default).

**NOTE**: Any use of a function which "warps" the cursor, moving it to
an absolute position, will break this undo stack.  Ideally this would
be fixed, but I've not yet looked at it properly.

(I suspect we can add a `WARP`-type to our undo-primitives which will
allow warping to be undone.  Tracked in [#14](https://github.com/skx/kilo/issues/14).)


## The Future

There are no obvious future plans, but [bug reports](https://github.com/skx/kilo/issues) may be made if you have a feature to suggest (or bug to report)!


Steve
--
https://steve.kemp.fi/
