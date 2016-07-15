Kilo
===

Kilo is a small text editor with around 1K lines of code (counted with cloc):

     -------------------------------------------------------------------
     Language        files          blank        comment           code
     -------------------------------------------------------------------
     C                   1            152            210           1089
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

Kilo does not depend on any library exclude for Lua5.2.

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

The following primitives are exported to lua:

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
* kill()
    * Kill the current line, deleting it from display.
* insert("string")
    * Inserts the given string at the current cursor position.  (Newlines work as expected.)
* open( [filename] )
    * Open the named file for reading.
    * If the filename is not given, prompt for one.
* page_up()
    * Scroll the screen up one page, if possible.
* page_down()
    * Scroll the screen down one page, if possible.
* save()
    * Save the file we're operating upon.
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
    * In the future this would be used to setup syntax highlighting.
* `on_saved(filename)`
    * Called __after__ a file is saved.
    * Can be used to make files executable, etc.


Copy & Paste
------------

`get_line` returns the text from the cursor to the end of the line, this is used to implement Copy & Paste in Lua - by default bound to `Ctrl-c` and `Ctrl-v`.

There is _also_ whole line copy&paste (named `kill` and `yank`) implemented as
in `nano` too which you'll find bound to `ctrl-y` and `ctrl-u`.  These only
operate upon a single line at a time though.


The Future?
-----------

Future plans?

* It might be nice to add accessors/mutators for the current cursor-position.

* The only outstanding task which is "obviously" a good idea is to move the definition of highlighting into Lua to allow swift addition of highlighting for #golang, #lua, etc.


Steve
--
https://steve.kemp.fi/
