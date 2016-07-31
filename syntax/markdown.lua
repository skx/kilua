--
-- Syntax Highlighting for markdown / text.
--
-- This mode only does two things:
--
--  * Highlights URLS.
--
--  * Highlights trailing whitespace.
--
-- Steve
--
--

--
-- This file is a Lua module.
--
local mymodule = {}

--
-- The string we return.
--
local retval = ""

--
-- Helper to add the colour.
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

local P = lpeg.P
local R = lpeg.R
local S = lpeg.S
local C = lpeg.C


-- Terminating characters for an URL.
local term  = S']> \n'

-- The two types of links we show.
local http  = P('http://') * (1 -term)^0/ function(...)  add(RED,...)end
local https = P('https://') * (1 -term)^0/ function(...) add(BLUE,...)end

-- Show trailing-whitespace with a `cyan` background.
local trailing_space = S' \t'^1 * S'\n'/ function(...) add(REV_CYAN,... ) end

-- Any character - allows continuation.
local any = C(P(1) )/ function(...) add(WHITE,... ) end

-- We support links and "any"thing else.
local tokens = (http + https + trailing_space + any )^0

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
