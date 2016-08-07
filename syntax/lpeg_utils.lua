--
-- This is a helper-library for LPEG-based parsing.
--


--
-- This file is a Lua module.
--
local mymodule = {}



--
-- Allow matching keywords
--
function mymodule.tokens(words)
   -- return value.
   local tmp = nil

   -- keywords are only a-z
   local idchars = lpeg.R("az")

   --
   -- Make a pattern that will match the given keyword
   --
   -- This is designed to match only on boundaries, but I suspect
   -- it is a little broken.
   --
   -- Reference
   --   http://stackoverflow.com/questions/38690698/using-lpeg-to-only-capture-on-word-boundaries/38767940#38767940
   --
   local function make_token(t)
      local nosuffix =  ( lpeg.P(t) * -idchars )
      local noprefix =  ( lpeg.B( 1 -idchars ) * ( lpeg.P(t) * -idchars ) )
      return( lpeg.C( noprefix * nosuffix^0 ) )
   end

   --
   -- For each kewyword .. append.
   --
   for i,o in ipairs(words) do
      if ( tmp ) then
         tmp = tmp + make_token(o)
      else
         tmp = make_token(o)
      end
   end
   return( tmp )
end


--
-- Helper function which returns something that matches "all the numbers"
--
function mymodule.numbers()
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

   return( hexnum + octnum + floatnum + decnum)
end

--
-- Export ourself
--
return mymodule
