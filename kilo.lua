
--
-- Keymap of bound keys
--
local keymap = {}

--
--  Default bindings
--
keymap['^A']        = sol
keymap['^E']        = eol
keymap['^D']        = function() insert( os.date() ) end
keymap['^S']        = save
keymap['PAGE_UP']   = page_up
keymap['PAGE_DOWN'] = page_down


--
-- Expand the given keyboard character.
--
--  e.g. "ctrl-a" -> "^A".
--
function expand_key(k)

   -- Convert to decimal
   local b = string.byte(k)

   -- Control-code
   if ( b < 32 ) then
      return( "^" .. ( string.char( b + string.byte("A" ) - 1 ) ) )
   end

   -- Normal ASCII
   if ( b < 128 ) then
      return k
   end

   -- These were learned by trial and error.
   if ( b == 237 )  then return "HOME"      end
   if ( b == 238 )  then return "END"       end
   if ( b == 239 )  then return "PAGE_UP"   end
   if ( b == 240 )  then return "PAGE_DOWN" end

   -- Expand for debugging
   k = "NONE-ASCII: " .. k .. " " .. string.byte(k) .. " ";
   return( k )
end


--
-- This function is called when input is received.
--
-- It will lookup the value of the keypress, and if it
-- represents a binding it will execute that function.
--
-- If no binding is found then the key is inserted into
-- the editor, literally.
--
function on_key(k)

   --
   -- Expand the character we've received to deal with
   -- control-codes, & etc.
   --
   k = expand_key(k)

   --
   -- Lookup the function in the key-map.
   --
   local func = keymap[k]

   --
   -- If found, execute it.
   --
   if ( func ~= nil ) then func() return end

   --
   -- Otherwise just insert the character.
   --
   insert(k)
end


--
-- Called at load-time
--
insert( "Steve Kemp added Lua!" )
