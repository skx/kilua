
function on_key(k)

   --
   -- Expand values less than "a" to be Ctrl-X.
   --
   if ( string.byte(k) < string.byte('a') ) then
      k = "^" .. ( string.char( string.byte(k) + string.byte('A')-1 ))
   end

   --
   -- Handle input
   --
   if ( k == "^A" ) then
      -- Ctrl-A -> Start of line
      sol()
   elseif ( k == "^E" ) then
      -- Ctrl-E -> End of line.
      eol()
   else
      -- Insert the character.
      insert(k)
   end
end


--
-- Called at load-time
--
insert( "Steve Kemp added Lua!" )
