--
-- Syntax Highlighting for INI files.
--
-- We only handle three cases
--
--  1.  Section is `[blah]` and is red.
--
--  2.  Key-name is "foo = " and is blue
--
--  3.  All other characters (i.e. key value) are white.
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


--
-- Sections.
--
local sect    = P'[' * (1 - P']')^0 * P']'
local section = (sect) / function(...)  add(RED, ... ) end

--
-- Key =
--
local key_name  = R"09" + R("az", "AZ") + S(' \t')
local key_rule  = key_name^1 * P'='^1
local key_match = (key_rule) / function(...)  add(BLUE, ... ) end

--
-- Show trailing-whitespace with a `cyan` background.
--
local trailing_space = S' \t'^1 * S'\n'/ function(...) add(REV_CYAN,... ) end

--
-- Match any single character: Which should be the value of the key.
--
local any = C(P(1) )/ function(...) add(WHITE,... ) end


--
-- The complete set of tokens we understand
--
local tokens = (section + key_match + trailing_space  + any)^0

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
