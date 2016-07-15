

--
-- How many times has Ctrl-q been pressed?
--
quit_count = 2

--
-- Called on quit.
--
--  * If the buffer is clean then exit immediately.
--  *  If the buffer is dirty require N Ctrl-q presses to exit.
--
function quit()
   if ( dirty() ) then
      if ( quit_count > 0 ) then
         status( "Buffer is dirty press Ctrl-q " .. quit_count .. " more times to exit")
         quit_count = quit_count - 1
      else
         exit()
      end
   else
      exit()
   end
end


--
-- Kill a line - by copying it to our kill-buffer, and deleting the line
--
kill_buffer = ""

function kill_line()
   -- move to beginning of line
   sol()
   -- get the line, and save it
   kill_buffer = get_line()
   -- kill the line
   kill()
end

--
-- Insert the kill-buffer at the point.
--
function yank()
   insert(kill_buffer .. "\n")
end


-- Paste-Buffer
--
paste_buffer = ""
function copy()
   paste_buffer = get_line()
end
function paste()
   insert( paste_buffer )
end

--
-- Keymap of bound keys
--
local keymap = {}

--
--  Default bindings
--
keymap['^A']        = sol
keymap['^D']        = function() insert( os.date() ) end
keymap['^E']        = eol
keymap['^H']        = delete
keymap['^K']        = kill_line
keymap['^L']        = eval
keymap['^M']        = function() insert("\n") end
keymap['^Q']        = quit
keymap['^O']        = open
keymap['^S']        = save
keymap['^T']        = function() status( os.date() ) end
keymap['^U']        = yank
keymap['BACKSPACE'] = delete
keymap['DEL']       = delete
keymap['PAGE_UP']   = page_up
keymap['PAGE_DOWN'] = page_down
keymap['LEFT']      = left
keymap['RIGHT']     = right
keymap['UP']        = up
keymap['DOWN']      = down


keymap['^C'] = copy
keymap['^V'] = paste


--
-- Expand the given keyboard character.
--
--  e.g. "ctrl-a" -> "^A".
--
function expand_key(k)

   -- Convert to decimal
   local b = string.byte(k)

   if ( b == nil ) then return "Ctrl-SPACE" end

   -- Control-code
   if ( b < 32 ) then
      if ( b == 9 ) then
         return "\t"
      end
      return( "^" .. ( string.char( b + string.byte("A" ) - 1 ) ) )
   end

   -- Normal ASCII
   if ( b < 127 ) then
      return k
   end

   -- These were learned by trial and error.
   if ( b == 127 )  then return "BACKSPACE" end
   if ( b == 232 )  then return "LEFT"      end
   if ( b == 233 )  then return "RIGHT"     end
   if ( b == 234 )  then return "UP"        end
   if ( b == 235 )  then return "DOWN"      end
   if ( b == 236 )  then return "DEL"       end
   if ( b == 237 )  then return "HOME"      end
   if ( b == 238 )  then return "END"       end
   if ( b == 239 )  then return "PAGE_UP"   end
   if ( b == 240 )  then return "PAGE_DOWN" end

   -- Expand for debugging
   k = "NON-ASCII: " .. k .. " " .. string.byte(k) .. " ";
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

   --
   -- If we reached here the previous character was not Ctrl-q
   -- so we reset the global-quit-count
   --
   quit_count = 2
end


--
-- This function is called when a file is loaded, and can
-- be used to setup syntax-highlighting in the future.
--
function on_loaded( filename )
end


--
-- This function is called AFTER a file is saved.
--
-- You might re-open the file and call `chmod 755 $file` if
-- the file has a shebang-line, or perform similar magic
-- here.
--
function on_saved( filename )

   --
   -- Process only the first line.
   --
   local n=0
   for l in io.lines(filename) do
      n=n+1
      if n==1 then
         if ( string.match(l, "^#!/" ) ) then
            -- shebang found
            status( "Shebang found in " .. filename .. " " .. l )
            os.execute( "chmod 755 " .. filename )
         end
         break
      end
   end
end
