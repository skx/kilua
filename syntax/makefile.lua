--
-- Trivial highlighting for Makefiles.
--



--
-- This file is a Lua module.
--
local mymodule = {}


--
-- Given a string split it into a table of lines, by the newline-character.
--
function string.to_table(str)
   local t = {}
   local function helper(line) table.insert(t, line) return "" end
   helper((str:gsub("(.-)\r?\n", helper)))
   return t
end


--
-- Does the string start with the given character-string?
--
function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end



--
-- The function we export.
--
function mymodule.parse(input)
   local ret = ""

   --
   -- Split the input by newlines.
   --
   tmp = string.to_table( input )

   --
   -- Iterate over every line.
   --
   for i,l in ipairs(tmp) do

      --
      -- Default colour
      --
      local colour = WHITE

      -- Starts with a comment?
      if (string.starts( l, "#" ) ) then
         colour = RED
      else
         -- Starts with a TAB?
         if (string.starts( l, '\t' ) ) then
            colour = BLUE
         else
            -- Is a rule ?
            if ( string.find(l, ":" ) ) then
               colour = YELLOW
            end
         end
      end

      --
      -- For each character in the line, set the colour.
      --
      len = #l
      while( len > 0 ) do
         ret = ret .. string.char( colour )
         len = len - 1
      end

      --
      -- Newline
      --
      ret = ret .. WHITE
   end

   return(tostring(ret))
end

--
-- Export ourself
--
return mymodule
