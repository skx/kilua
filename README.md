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
* insert("string")
    * Inserts the given string at the current cursor position.  (Newlines work as expected.)
* page_up()
    * Scroll the screen up one page, if possible.
* page_down()
    * Scroll the screen down one page, if possible.
* save()
    * Save the file we're operating upon.
* sol()
    * Move the cursor to the start of the current line.

In addition to those functions there are also the movement-related primitives: up(), down(), left(), right().

Finally `get_line` returns the text from the cursor to the end of the line, this is used to implement Copy & Paste in Lua - by default bound to `Ctrl-c` and `Ctrl-v`.

Future plans?  It might be nice to add accessors/mutators for the current
cursor-position, and allow Lua to populate the status-buffer.  Otherwise
I'll probably stop here.
