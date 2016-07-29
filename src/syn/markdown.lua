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

-- local lpeg = require('lpeg')
--
-- This file is a Lua module.
--
local mymodule = {}

--
--  Start of position.
--
local start = 0



--
-- Helper to add the colour.
--
function add( colour, str )
   length = string.len(str)
   syntax_range( start, start+length, colour )
   start = start + length
end

local P = lpeg.P
local R = lpeg.R
local S = lpeg.S
local C = lpeg.C



-- Terminator for a URL
local term  = S']> \n'/ function(...) add(8,... ) end

-- Two types of links
local http  = P('http://') * (1 -term)^0/ function(...)  add(4,...)end
local https = P('https://') * (1 -term)^0/ function(...) add(4,...)end

-- Any character - allows continuing.
local any   = C(P(1) )/ function(...) add(8,... ) end


-- We support links and "any"thing else.
local tokens = (http + https + any )^0

--
-- The function we export.
--
function mymodule.parse(input)
   start = 0
   lpeg.match(tokens, input)
end

--
-- Export ourself
--
return mymodule
