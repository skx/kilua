--
-- Syntax Highlighting for markdown - this is a faux mode that
-- only attempts to highlight URLS.
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
-- The string we return.
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

local P = lpeg.P
local R = lpeg.R
local S = lpeg.S
local C = lpeg.C



-- Terminator for a URL
local term  = S']> \n'/ function(...) add(WHITE,... ) end

-- Two types of links
local http  = P('http://') * (1 -term)^0/ function(...)  add(RED,...)end
local https = P('https://') * (1 -term)^0/ function(...) add(BLUE,...)end

-- Any character - allows continuing.
local any   = C(P(1) )/ function(...) add(WHITE,... ) end


-- We support links and "any"thing else.
local tokens = (http + https + any )^0

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
