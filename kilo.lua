
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
keymap['PAGE_UP']   = page_up
keymap['PAGE_DOWN'] = page_down


function page_up()
   insert( "PAGE UP" )
end
function page_down()
   insert( "PAGE DOWN" )
end

--
-- Expand values less than "a" to be Ctrl-X.
--
-- TODO: Expand PAGE_UP, ARROW_UP, etc.
--
function expand_key(k)
   if ( string.byte(k) < string.byte('a') ) then
      k = "^" .. ( string.char( string.byte(k) + string.byte('A')-1 ))
      return k
   end

   --
   -- Convert to int and look for special characters
   --
   local b = string.byte(k)
   if ( b == 239 )  then return "PAGE_UP"   end
   if ( b == 240 )  then return "PAGE_DOWN" end

   return k
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
