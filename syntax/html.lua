--
-- Syntax Highlighting for HTML
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
-- Character-strings
--
local string  = P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'
local strings = (string) / function(...) add(CYAN, ... ) end

--
-- Comments.
--
local html_comment = P'<!-- ' * (1 - P'-->')^0 * P'-->'
local comments     = (html_comment) / function(...)  add(YELLOW, ... ) end

--
-- Show trailing-whitespace with a `cyan` background.
--
local trailing_space = S' \t'^1 * S'\n'/ function(...) add(REV_CYAN,... ) end

--
-- Tags
--
local tagged = P'<' * (1 - P'>')^0 * P'>'
local tags   = (tagged) / function(...)  add(BLUE, ... ) end

--
-- Match any single character
--
local any = C(P(1) )/ function(...) add(WHITE,... ) end


--
-- The complete set of tokens we understand
--
local tokens = (strings + comments + trailing_space + tags + any)^0

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
