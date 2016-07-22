# Lua Primitives

This document briefly describes each of the available Lua primitives
which are implemented in the editor.

## Buffers

* `buffers()`
* `choose_buffer()`
* `create_buffer()`
* `current_buffer()`
* `kill_buffer()`
* `next_buffer()`
* `prev_buffer()`
* `select_buffer()`


## Core Functions

* `dirty()`
    * Is the current buffer modified & unsaved?
* `exit()`
    * Exit the editor.
* `filename()`
    * Get/Set the filename of the current buffer.
* `find()`
    * Open and interactive find mode, for performing forward/backward searches.
* `height()`
    * Return the height of the terminal we're running in.
* `open([filename])`
    * Open a file, and insert the text into the current buffer.
* `save([filename])`
    * Save the current buffer.
    * If there is a filename given this will be used.
* `search(regexp)`
    * Search forward for the given regular expression.
* `status()`
    * Set the contents of the status-bar.
* `undo()`
    * Undo the previous action(s).
* `width()`
    * Return the width of the terminal we're running in.


## Movement

* `down()`
    * Move the cursor down one character.
* `eol()`
    * Move to the end of the current line.
* `left()`
    * Move the cursor one character to the left.
* `page_up()`
    * Scroll the screen up one page, if possible.
* `page_down()`
    * Scroll the screen down one page, if possible.
* `right()`
    * Move the cursor one character to the right.
* `sol()`
    * Move the cursor to the start of the current line.
* `up()`
    * Move the cursor one character up.


## Insertions & Removals & Accessors

* `at(x,y)`
    * Return the character at the given point.
* `delete()`
    * Delete a single character (backwards).
* `get_line()`
    * Retrieve the text from the point to the end of the line.
* `prompt( "prompt" )`
    * Prompt the user for input, and return it to lua.
* `key()`
    * Read a single key from the keyboard (blocking).
    * Used to implement setting of marks.
* `kill()`
    * Kill the current line, deleting it from display.
* `insert("string")`
    * Inserts the given string at the current cursor position.
    * (Newlines work as expected.)
* `text()`
    * Return the text in the buffer.

## Marks

* `mark()`
    * Get/Set the X,Y position of the global mark, which allows a selection to be made.
    * The selection is the region between the cursor/point and the mark.
* `point()`
    * Get/Set the X,Y coordinate of the cursor.


## Selection

Functions relating to the selection.

* `cut_selection()`
   * Remove the selected text from the buffer.
* `selection()`
   * Retrieve the contents of the selected region.


## Syntax Highlighting.

* `set_syntax_comments()`
* `set_syntax_keywords()`
* `syntax_highlight_numbers()`
* `syntax_highlight_strings()`
* `tabsize()`
    * Get/Set the width of TAB-stops (eight characters by default).
