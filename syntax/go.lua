--
-- Syntax Highlighting for go
--



--
-- Require our helper-library
--
local lpeg_utils = require( "lpeg_utils" )


--
-- This file is a Lua module.
--
local mymodule = {}

--
-- The result we return to the caller.
--
local retval = ""

--
-- Helper to add the colour.
--
function add( colour, str )
   length = string.len(str)
   while( length >0 ) do
      retval = retval .. string.char( colour )
      length = length -1
   end
end

--
-- Shorten our references.
--
local P = lpeg.P
local R = lpeg.R
local S = lpeg.S
local C = lpeg.C



--
-- Numbers
--
local numbers = lpeg_utils.numbers() / function(...) add(YELLOW, ... ) end


--
-- Character-strings
--
local charlit   = P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"
local stringlit = P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'
local strings   = (charlit + stringlit) / function(...) add(BLUE, ... ) end


--
-- Single and multi-line comments.
--
local ccomment   = P'/*' * (1 - P'*/')^0 * P'*/'
local newcomment = P'//' * (1 - P'\n')^0
local comment    = (ccomment + newcomment) / function(...)  add(RED, ... ) end

--
-- Show trailing-whitespace with a `cyan` background.
--
local trailing_space = S' \t'^1 * S'\n'/ function(...) add(REV_CYAN,... ) end

--
-- Literals
--
local literal = ( charlit + stringlit) / function(...) add(BLUE, ... ) end

--
-- Keywords
--
local keyword = lpeg_utils.tokens({

      -- real keywords
      'break',
      'case',
      'chan',
      'const',
      'continue',
      'default',
      'defer',
      'else',
      'fallthrough',
      'for',
      'func',
      'go',
      'goto',
      'if',
      'import',
      'interface',
      'map',
      'package',
      'range',
      'return',
      'select',
      'struct',
      'switch',
      'type',
      'var'

      -- types
      'bool',
      'byte',
      'complex64',
      'complex128',
      'error',
      'float32',
      'float64',
      'int',
      'int8',
      'int16',
      'int32',
      'int64',
      'rune',
      'string',
      'uint',
      'uint8',
      'uint16',
      'uint32',
      'uint64',
      'uintptr',
} ) / function(...) add(CYAN, ... ) end

--
-- Functions :
--
local functions = lpeg_utils.tokens({

      -- misc
      'true',
      'false',
      'iota',
      'nil',

      -- real functions
      'append',
      'cap',
      'close',
      'complex',
      'copy',
      'delete',
      'imag',
      'len',
      'make',
      'new',
      'panic',
      'print',
      'println',
      'real',
      'recover'
} ) / function(...) add(GREEN, ... ) end


--
-- Match any single character
--
local any = C(P(1) )/ function(...) add(WHITE,... ) end



--
-- The complete set of tokens we understand
--
local tokens = (comment + functions + keyword + numbers + strings + trailing_space + any)^0


--
-- The function we export.
--
function mymodule.parse(input)
   retval = ""
   lpeg.match(tokens, input)
   return( retval )
end

--
-- Export ourself
--
return mymodule
