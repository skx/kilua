
Syntax Highlighters
-------------------

These use `lpeg`.

A buffer has an associated syntax-mode, if a mode is set then
the module `${mode}.lua` will be loaded, and the call-back function
`on_syntax_highlight(text)` will be called.


## on_syntax_hightlight

This function is expected to return a **string**, which will contain
one character for each byte of the input.

Given the input string "foo" the output string should encode the
colour to use for the given offset.  For example:

     on_syntax_highlight( "foo" ) -> "888"
     -> Draw each character in white.

     on_syntax_highlight( "stevekemp" ) -> "123456788"
     -> Draw each charcter in a different colour.


**TODO**:

* There are eight colours.
* Write more documentation.
