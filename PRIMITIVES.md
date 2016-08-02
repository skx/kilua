# Lua Primitives

This document briefly describes each of the available Lua primitives
which are implemented in the editor.

Most of these primitives are demonstrated in the default [kilua.lua](https://github.com/skx/kilua/blob/master/kilua.lua) file.



## Buffer Primitives

* `buffer`
    * Get or set the current buffer.
    * `buffer(1)` will select buffer 1.
    * `buffer("name")` will select the buffer with the given name, returning `-1` if it doesn't exist.
    * `buffer()` returns the index of the currently selected buffer.
* `buffers()`
    * Return a table of all known buffers.
    * The table will contain the integer-offset, along with the name.
* `buffer_name()`
    * Get/Set the name of the current buffer.
* `create_buffer()`
    * Create a buffer with the given name.
* `kill_buffer()`
    * Delete the currently selected buffer.



## Core Primitives

* `delete()`
    * Delete a single character, to the left of the point.
    * See `delete_forwards()` in the default configuration file for the reverse.
* `dirty()`
    * Is the current buffer modified & unsaved?
* `exit()`
    * Exit the editor, immediately.
* `insert(string)`
    * Insert the given string into the current buffer.
* `key()`
    * Read a single (wide) key from the user.
* `menu()`
    * Given a table of strings allow the user to choose one of them, returning the index of the selected choice.
* `point()`
    * Get/Set the position of the cursor/point.
* `prompt( message )`
    * Prompt the user for a line of input, showing the specified message.
* `open([filename])`
    * Open a file, and insert the text into the current buffer.
* `save([filename])`
    * Save the current buffer.
    * If there is a filename given this will be used.
* `search(regexp)`
    * Search forward for the given regular expression.
* `status(msg)`
    * Set the contents of the status-bar.


## File Primitives

We only need two primitives so far for dealing with the filesystem:

* `directory_entries(path)`
     * Return a table containing the names of all files beneath the given directory.
     * For example `directory_entries("/etc/")`
* `exists(path)`
     * Return `true` if the given path exists, `false` otherwise.


## Movement Primitives

* `move(direction)`
    * This function moves the cursor, if possible.
    * For example `move('left')`, `move("right")`, etc.
* `eof()`
    * Move to the end of the current file/buffer.
* `eol()`
    * Move to the end of the current line.
* `sof()`
    * Move to the start of the current file/buffer.
* `sol()`
    * Move to the start of the current line.

See `page_up` and `page_down` in the default configuration file for examples
of movement facilities built upon this one.


## Screen Primitives

* `at(x,y)`
    * Return the (wide) character at the given position, if it exists.
* `height()`
    * Return the height of the editor-area.  This is the same as the screen height, minus two lines to account for the status-area.
* `width()`
    * Return the width of the editor-area.



## Syntax Highlighting Primitives

* `syntax()`
    * Get/Set the syntax-mode.
