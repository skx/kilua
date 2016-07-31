--
-- Syntax Highlighting for Lua
--


--
-- This file is a Lua module.
--
local mymodule = {}

--
-- The result we return to the caller.
--
local retval = ""


--
-- Helper to add the colouring.
--
function add( colour, str )
   length = string.len(str)
   while( length >0 ) do
      -- The colour has "0" subtracted from it.
      local val = ( string.char( ( string.byte('0') + colour ) ) )
      retval = retval .. val
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


--
-- Single & multi-line comments.
--
local ccomment   = P'--[[' * (1 - P'--]]')^0 * P'--]]'
local newcomment = P'--' * (1 - P'\n')^0
local comment    = (ccomment + newcomment) / function(...)  add(RED, ... ) end

-- Show trailing-whitespace with a `cyan` background.
local trailing_space = S' \t'^1 * S'\n'/ function(...) add(REV_CYAN,... ) end

-- Literals
local literal = (numlit + charlit + stringlit) / function(...) add(BLUE, ... ) end

-- Keywords
local keyword = C(
   P"and" +
   P"break" +
   P"do" +
   P"else" +
   P"elseif" +
   P"end" +
   P"false" +
   P"for" +
   P"function" +
   P"goto" +
   P"if" +
   P"in"+
   P"local" +
   P"nil" +
   P"not" +
   P"or" +
   P"repeat" +
   P"return" +
   P"then"+
   P"true" +
   P"until" +
   P"while"
                 ) / function(...) add(CYAN, ... ) end


-- Functions
local functions = C(
   P"load" +
   P"require" +
   P"pairs" +
   P"ipairs" +
   P"pairs" +
   P"tonumber" +
   P"tostring" +
   P"print" +
   P"type"
                   ) / function(...) add(BLUE, ... ) end


--
-- Match any single character
--
local any = C(P(1) )/ function(...) add(WHITE,... ) end


--
-- The complete set of tokens we understand
--
local tokens = (comment + keyword + functions + literal + trailing_space  + any)^0

--
-- The function we export.
--
function mymodule.parse(input)
   retval = ""
   lpeg.match(tokens, input)
   return(retval)
end

--
-- Export ourself
--
return mymodule
