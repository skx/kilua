--
-- Syntax Highlighting for C/C++
--
--        1,  COLOR_RED
--        2,  COLOR_GREEN
--        3,  COLOR_YELLOW
--        4,  COLOR_BLUE
--        5,  COLOR_MAGENTA
--        6,  COLOR_CYAN
--        7,  COLOR_BLUE
--        8,  COLOR_WHITE
--
--


--
-- This file is a Lua module.
--
local mymodule = {}

--
--  Start of position.
--
local start = 0

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
      retval = retval .. tostring(colour)
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
-- Whitespace isn't syntax highlighted, but we still have to
-- account for it in our length calculations.
--
local white = S' \t\v\n\f'
local whitespace = (white) / function(...) add(6, ...) end


local digit = R'09'
local letter = R('az', 'AZ') + P'_'
local alphanum = letter + digit
local hex = R('af', 'AF', '09')
local exp = S'eE' * S'+-'^-1 * digit^1
local fs = S'fFlL'
local is = S'uUlL'^0

local hexnum = P'0' * S'xX' * hex^1 * is^-1
local octnum = P'0' * digit^1 * is^-1
local decnum = digit^1 * is^-1
local floatnum = digit^1 * exp * fs^-1 +
   digit^0 * P'.' * digit^1 * exp^-1 * fs^-1 +
   digit^1 * P'.' * digit^0 * exp^-1 * fs^-1
local numlit = hexnum + octnum + floatnum + decnum


-- Character-string
local charlit =
   P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"

-- String.
local stringlit =
   P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'


-- Multi-line comment
local ccomment = P'/*' * (1 - P'*/')^0 * P'*/'
-- Single-line comment
local newcomment = P'//' * (1 - P'\n')^0

-- Both our comments.
local comment = (ccomment + newcomment) / function(...)  add(1, ... ) end

-- Literals
local literal = (numlit + charlit + stringlit) / function(...) add(4, ... ) end

-- Keywords
local keyword = C(
      P"auto" +
      P"_Bool" +
      P"break" +
      P"case" +
      P"char" +
      P"_Complex" +
      P"const" +
      P"continue" +
      P"default" +
      P"do" +
      P"double" +
      P"else" +
      P"enum" +
      P"extern" +
      P"float" +
      P"for" +
      P"goto" +
      P"if" +
      P"_Imaginary" +
      P"inline" +
      P"int" +
      P"long" +
      P"register" +
      P"restrict" +
      P"return" +
      P"short" +
      P"signed" +
      P"sizeof" +
      P"static" +
      P"struct" +
      P"switch" +
      P"typedef" +
      P"union" +
      P"unsigned" +
      P"void" +
      P"volatile" +
      P"while"
                 ) / function(...) add(4, ... ) end

-- Functions
local functions = C(
   P"stderr" +
   P"stdout" +
   P"printf" +
   P"strlen" +
   P"fprintf" +
   P"strcpy" +
   P"strncpy" +
   P"sprintf" +
      P"getenv" +
      P"ioctl" +
      P"lua_register"+
      P"luaL_newstate"+
      P"luaopen_base"+
      P"luaL_openlibs"
                   ) / function(...) add(5, ... ) end


--
-- Match any single character
--
local any = C(P(1) )/ function(...) add(8,... ) end


--
-- The complete set of tokens we understand
--
local tokens = (comment + functions + keyword + literal + whitespace + any)^0


--
-- The function we export.
--
function mymodule.parse(input)
   start  = 0
   retval = ""
   lpeg.match(tokens, input)
   return( retval )
end

--
-- Export ourself
--
return mymodule
