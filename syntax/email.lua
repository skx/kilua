--
-- Trivial highlighting for emails
--
-- This does not use LPEG, instead it parses line-by-line, as this
-- is simpler.
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
   -- We default to being in the message-header, not the
   -- message signature (which might not even be present).
   --
   local header = true
   local sig = false

   --
   -- Iterate over every line.
   --
   for i,l in ipairs(tmp) do

      --
      -- If we're in the header then we're blue
      --
      if ( header ) then

         len = #l
         while( len > 0 ) do
            ret = ret .. BLUE
            len = len - 1
         end

         --
         -- Is this the end of a header?
         --
         if ( l == "" ) then
            header = false
         end
      else

         --
         -- If we're in the signature we're yellow
         --
         if ( sig ) then

            len = #l
            while( len > 0 ) do
               ret = ret .. YELLOW
               len = len - 1
            end
         else

            --
            -- Otherwise we're in the body, and we default to
            -- white, unless we're seeing quoted-lines
            --

            --
            -- Default colour for body.
            --
            local colour = WHITE

            --
            -- Quotes
            --
            if ( string.starts( l, ">" ) )    then  colour = RED    end
            if ( string.starts( l, "> " ) )   then  colour = RED    end
            if ( string.starts( l, "> > " ) ) then  colour = GREEN  end
            if ( string.starts( l, ">> " ) )  then  colour = GREEN  end

            --
            -- Body
            --
            len = #l
            while( len > 0 ) do
               ret = ret .. string.char(colour)
               len = len - 1
            end

            --
            -- Are we at the signature?
            --
            if ( l == "--" ) or ( l == "-- " ) then
               sig = true
            end
         end
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
