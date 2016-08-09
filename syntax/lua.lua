--
-- Syntax Highlighting for Lua
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
-- Helper to add the colouring.
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
-- Single & multi-line comments.
--
local ccomment   = P'--[[' * (1 - P'--]]')^0 * P'--]]'
local newcomment = P'--' * (1 - P'\n')^0
local comment    = (ccomment + newcomment) / function(...)  add(RED, ... ) end

--
-- Show trailing-whitespace with a `cyan` background.
--
local trailing_space = S' \t'^1 * S'\n'/ function(...) add(REV_CYAN,... ) end

--
-- Keywords
--
local keyword = lpeg_utils.tokens({
                                     "and",
                                     "break",
                                     "do",
                                     "else",
                                     "elsif",
                                     "end",
                                     "false",
                                     "for",
                                     "function",
                                     "goto",
                                     "if",
                                     "in",
                                     "local",
                                     "nil",
                                     "not",
                                     "or",
                                     "repeat",
                                     "return",
                                     "then",
                                     "true",
                                     "until",
                                     "while"
                                  }) / function(...) add(CYAN, ... ) end


--
-- Functions from the standard-library
--
local func = {
   "assert",
   "ipairs",
   "load",
   "pairs",
   "print",
   "require",
   "tonumber",
   "tostring",
   "type"
}

--
--  Add the functions from the standard-packages to our function-table
--
local packages = {
   "io",
   "math",
   "os",
   "string",
   "table"
}
for i,n in pairs(packages) do
   for k,v in pairs(_G[n]) do
      table.insert( func,  n .. "." .. k)
   end
end

--
-- Now highlight the functions
--
local functions = lpeg_utils.tokens(func)  / function(...) add(GREEN, ... ) end


--
-- Match any single character
--
local any = C(P(1) )/ function(...) add(WHITE,... ) end


--
-- The complete set of tokens we understand
--
local tokens = (comment + keyword + functions + strings + numbers + trailing_space  + any)^0

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
