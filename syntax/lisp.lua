--
-- This is a simple "syntax highlighter" - it just highlights
-- parenthesis.
--
-- It is designed to demonstrate how you don't need to use LPEG
-- if you don't want
--


--
-- This file is a Lua module.
--
local mymodule = {}


--
-- The function we export.
--
function mymodule.parse(input)
   local ret = ""

   for letter in input:gmatch(".") do
      if ( letter == '(' or letter == ')' ) then
         ret = ret .. CYAN
      else
         ret = ret .. WHITE
      end
   end
   return(tostring(ret))
end

--
-- Export ourself
--
return mymodule
