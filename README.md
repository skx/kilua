[![Build Status](https://travis-ci.org/skx/kilua.png)](https://travis-ci.org/skx/kilua)
[![license](https://img.shields.io/github/license/skx/kilua.svg)]()


# Kilua

Kilua is a Lua-powered text editor written in around 2K lines of code, which
was forked from the minimal [kilo editor](https://github.com/antirez/kilo)
originally written by @antirez, and [introduced here on his blog](http://antirez.com/news/108).

You can launch the editor via :

    $ kilua [options] /path/to/file

Once launched the arrow keys will move you around, and the main keybindings
to learn are:

    CTRL-l: Read lua from the user, and evaluate it.
    CTRL-o: Open a new file.
    CTRL-s: Save the current file.
    CTRL-q: Quit
    CTRL-f: Find string in file (ESC to exit search, arrows to navigate)

Currently supported command-line options are:

* `--config file`
    * Load the named (lua) configuration file, in addition to the defaults.
* `--eval`
    * Evaluate the given lua, post-load.
* `--version`
    * Report the version and exit.


## Lua Support

This project was updated by [Steve Kemp](https://steve.kemp.fi/)
to include a lua intepreter, and delegate as much as possible of the operation
to that lua instance.

* On startup our initialization files are read:
    * `~/.kilua.lua`.
    * `./kilua.lua`.
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
* search( "string" )
    * Search forward for the given text.
    * Returns the length of the match on success, zero on failure.
* selection()
    * Return the text between cursor and mark.
* set_syntax_comments()
    * Setup comment-handling for syntax-highlighting.
* set_syntax_keywords()
    * Setup keyword-handling for syntax-highlighting.
* sol()
    * Move the cursor to the start of the current line.
* status("text-message")
    * Show the given message in the status-area.
* tabsize()
    * Get the width of TAB characters (default 8).
* tabsize(N)
    * Set the width of TAB characters.

In addition to those functions there are also the obvious movement-related primitives: `up()`, `down()`, `left()`, and `right()`.


## Callbacks

There is room for lots more functionality to be delegated to callbacks,
but right now there are only these:

* `on_idle()`
    * Called roughly once a second, can be used to run background things.
* `on_key(key)`
    * Called to process a single key input.
* `on_loaded(filename)`
    * Called when a file is loaded.
    * This sets up syntax highlighting in our default implementation for C and Lua files.
* `on_saved(filename)`
    * Called __after__ a file is saved.
    * Can be used to make files executable, etc.



## Buffers

`kilua` allows multiple files to be opened, via the use of buffers.  If `kilua` is launched without any filename parameters there will be two buffers:

* `*Messages*`
    * This receives copies of the status-message.
* An unnamed buffer for working with.
    * Enter your text here, then use `M-x save("name")` to save it.

Otherwise there will be one buffer for each file named upon the command-line,
as well as the `*Messages*` buffer.  (You can kill the `*Messages*` buffer
if you wish, but it's a handy thing to have around.)

The default key-bindings for working with buffers are:

Action                             | Binding
---------------------------------- | --------------
Create a new buffer.               | `Ctrl-x c`
kill the current buffer.           | `Ctrl-x k`
kill the current buffer, forcibly. | `Ctrl-x K`
Select the next buffer.            | `Ctrl-x n` or `M-right`
Select the previous buffer.        | `Ctrl-x p` or `M-left`
Choose a buffer, via menu.         | `Ctrl-x b` or `Ctrl-x B`

It's worth noting that you can easily create buffers dynamically, via lua, for
example the following function can be called by `M-x uptime()`, and does
what you expect:

* Select the buffer with the name `*uptime*`.
     * If that buffer doesn't exist then create it.
* Move to the end of the buffer.
     * Insert the output of runing `/usr/bin/uptime` into the buffer.


      -- Run `uptime`, and show the result in a dedicated buffer.
      function uptime()
          local result = select_buffer( "*uptime*" )
          if ( result == 0 ) then create_buffer( "*uptime*" )   end
          end_of_file()
          insert(cmd_output( "uptime" ) )
      end


## Copy & Paste

We've added a notion of a `mark`.  A mark is set by pressing `Ctrl+space`,
and at any time you can cut the region between the cursor and the mark by
pressing `Ctrl-w`.

You can also cut the current line via `Ctrl-y`.

In both cases you can yank the selection back with `Ctrl-u`.


## Marks

You can record your position (i.e. "mark") in a named key, and
later jump to it, just like in `vi`.

To record the current position use `M-SPACE`, and press the key
you wish to use.  To return to it use `M-m XX` where XX was the
key you chose.


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

**NOTE**: Due to the implementation some actions might corrupt the
undo-stack.  Specifically difficult functions are:

* `kill`.
* `cut_selection`.

## The Future

There are no obvious future plans, but [bug reports](https://github.com/skx/kilua/issues) may be made if you have a feature to suggest (or bug to report)!


Steve
--
https://steve.kemp.fi/
