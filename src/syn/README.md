
Syntax Highlighters
-------------------

These use `lpeg`.

A buffer has an associated syntax-mode, if a mode is set then
the module `${mode}.lua` will be loaded, and the call-back function
`on_syntax_highlight(text)` will be called.

To add syntax highlighting your callback function must invoke the
helper function:

`syntax_range( start, end , colour`

This will set teh region between start + end (inclusive) to the given
colour.

More documentation is required.
