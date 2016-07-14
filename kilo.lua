
function on_key(k)
   if ( string.byte(k) == 1 ) then
      sol()
   elseif ( string.byte(k) == 5 ) then
      eol()
   else
      insert(k)
   end
end


--
-- Called at load-time
--
insert( "Steve Kemp added Lua!" )
