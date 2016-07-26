--
--
-- This is the configuration file for kilua.
--
--  When kilua starts it will look for two default configuration-files:
--
--   ~/.kilua/init.lua
--   ~/.kilua/$(hostname).lua
--
-- If neither file is present then _this_ file will be loaded, as it is
-- embedded into the generated binary, via the file `config.h`, as part
-- of the build-process.
--
-- There are several functions that kilua invokes at various times, without
-- which you'll have no functionality, so a configuration file is required:
--
--  * get_status_bar()
--     This function is called to populate the status-bar in the footer.
--
--  * on_idle(key)
--     Called when things are idle, to allow actions to be carried out.
--
--  * on_key(key)
--     Called when input is received.
--
-- Otherwise the only magic here is the `keymap` table.  Kilua will lookup
-- every keypress in this table, and if there is a matching function defined
-- it will be invoked, otherwise the literal character will be inserted.
--
-- Steve
--
-- https://steve.kemp.fi/



--
-- Keybinding Settings.
--
------------------------------------------------------------------------
--======================================================================
------------------------------------------------------------------------

--
-- Table of bound keys
--
local keymap = {}

--
--  Key bindings.
--
--  You'll see some bindings use "function() ... end" this is to cope
-- with the fact that the function they invoke is not yet defined, because
-- it comes later in this file.
--
--  For example this would fail:
--
--     keymap['^Q'] = quit
--
--  But by the time this function is _completely_ loaded the function is
-- indeed defined which allows this to work:
--
--     keymap['^Q'] = function() quit() end
--
--
keymap['ENTER']    = function() insert("\n") end
keymap['q']        = function() exit() end
-- TODO: keymap['^ ']        = function() set_mark() end
keymap['^A']        = sol
keymap['^B']        = function() local x = os.date() insert(x) ; end
keymap['^D']        = function() insert(os.date()) end
keymap['^E']        = eol
keymap['^F']        = find
keymap['^H']        = delete
-- TODO: keymap['^_']        = undo
-- TODO: keymap['^G']        = function() search( prompt( "Search:" ) ) end
-- TODO: keymap['^J']        = function() goto_mark() end
-- TODO: keymap['^K']        = function() kill_line() end
-- TODO: keymap['^N']        = function() record_mark() end
keymap['^Q']        = function() quit() end
--TODO: keymap['^T']        = function() status( os.date() ) end
--TODO: keymap['^U']        = function() yank() end
-- TODO: keymap['^W']        = function() kill_between_point_and_mark() end
-- TODO: keymap['^Z']        = undo
keymap['KEY_BACKSPACE'] = delete
keymap['KEY_PPAGE']   = page_up
keymap['KEY_NPAGE'] = page_down
keymap['KEY_LEFT']      = function() move("left") end
keymap['KEY_RIGHT']     = function() move("right") end
keymap['KEY_UP']        = function() move("up") end
keymap['KEY_DOWN']      = function() move("down") end

--
-- ASCII testing:
--
--  INSERT / DELETE insert a key.
--
--  We've also bound pi/euro characters to run code.
--
keymap['KEY_IC'] = function() insert( "π" ) end
keymap['KEY_DC'] = function() insert( "€" ) end
keymap['€']      = function() status("Euros are like pounds, but smaller!") end
keymap['π']      = function() status("Mmmm, pie ..") end

--
-- Home/End goes to start/end of line
--
-- Esc-Home/Esc-End goes to start/end of file
--
keymap['KEY_HOME']      = sol
keymap['KEY_END']       = eol
keymap['M-KEY_HOME']    = sof
keymap['M-KEY_END']     = eof

--
-- M-x -> eval, just like emacs.
--
keymap['M-x'] = function() eval_lua() end

--
-- M-! ("Escape", then "!") will run a command and insert the output
-- into your document
--
keymap['M-!'] = function() cmd = prompt( "execute:" ); if ( cmd ) then insert( cmd_output(cmd) ) end end

--
-- M-SPACE will record a mark.
--
keymap['M- '] = function() record_mark() end

--
-- Prefix-map for jumping to recorded marks.
--
keymap['M-m'] = {}


--
-- Prefixed keybindings
--
--  ^X ^S => Save
--
--  ^X ^C => Exit
--
--  ^X ^X => Swap point and mark
--
keymap['^X'] = {}
keymap['^X']['^C'] = function() quit() end
keymap['^X']['^S'] = save
-- TODO: keymap['^X']['^X'] = function() swap_point_mark() end
keymap['^X']['i']  = function() insert_contents() end

--
-- Working with buffers.
--
-- TODO: keymap['^X']['b']  = choose_buffer
-- TODO: keymap['^X']['B']  = choose_buffer
keymap['^X']['K']  = kill_buffer
keymap['^X']['c']  = create_buffer
keymap['^X']['k']  = function() confirm_kill_buffer() end
keymap['M-KEY_RIGHT']  = function() next_buffer() end
keymap['^X']['n']  = function () next_buffer() end
keymap['M-KEY_LEFT']   = function() prev_buffer() end
keymap['^X']['p']  = function() prev_buffer() end


--
--  Callbacks and utilities they use.
-----------------------------------------------------------------------------



--
-- This function is called when input is received.
--
-- It will lookup the value of the keypress, and if it
-- represents a binding it will execute that function.
--
-- If no binding is found then the key is inserted into
-- the editor, literally.
--
do
   pending_char = nil
   pending_esc  = false

   function on_key(k)

      --
      -- Was this escape?
      --
      -- Special case if it was ESC-ESC
      --
      if ( k == "ESC" ) then
         if ( pending_esc ) then
            k = "M-ESC"
            pending_esc = false
         else
            pending_esc = true
            return
         end
      end

      --
      -- Pending-escape?
      --
      if ( pending_esc == true ) then
         k = "M-" .. k
         pending_esc = false
      end

      --
      -- At this point the key didn't result in a function.
      --
      -- Nor did it terminate a special action.
      --
      -- So we'll now look for the expanded function.
      --
      if ( pending_char ) then
         local val = keymap[pending_char][k]

         if ( type(val) == 'function' ) then
            --
            -- We got a function to invoke!
            --
            val()
            pending_char = nil
            return
         else
            --
            -- Failure
            --
            pending_char = nil
            return
         end
      end


      --
      -- Lookup the key in our key-map.
      --
      local result = keymap[k]
      status( "Key " .. k .. " result:" .. type(result))
      --
      -- Was there a result?
      --
      if ( result ) then

         if ( type(result) == 'function' ) then
            result()
            pending_esc = false
            pending_char  = nil
            return
         end
         if ( type(result) == 'table' ) then

            --
            -- This is a pending multi-part key - record this part away.
            --
            pending_esc = false
            pending_char = k
            return
         end
      end

      --
      -- We should ignore escape-characters and ctrl-characters
      -- here.
      --
--      if ( #k > 1 ) then
--         status("Ignoring special-character which isn't bound : '" .. k  .. "'")
--         return
--      end

      --
      -- Otherwise just insert the character.
      --
      insert(k)
      pending_char = nil
      quit_count   = 2
   end
end


--
-- This function is called when a file is loaded, and is used
-- to setup the syntax highlighting.
--
function on_loaded( filename )

   if ( not filename ) then return end

   -- Get the file-name + suffix.
   local file = filename:match("^.+/(.+)$") or filename
   local ext = file:match("^.+%.(.+)$") or file

   -- Lookup the entry
   local syntax = syn[ext] or syn[file]

   -- If that worked, and there are keywords..
   if (syntax and syntax['keywords'] ) then

      -- Set the keywords
      set_syntax_keywords( syntax['keywords'] )

      -- If there are defined values for the comments, set them too.
      if ( syntax['single'] and
           syntax['multi_open'] and
           syntax['multi_close'] ) then
         set_syntax_comments(syntax['single'],syntax['multi_open'], syntax['multi_close'] )
      end
   end

   -- Set any options, if we found them.
   if (syntax and syntax['options'] ) then
      set_syntax_options( syntax['options'] )
   end

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


--
--  Utility functions.
-----------------------------------------------------------------------------


function set_syntax( name )
   -- Lookup the entry
   local syntax = syn[name]
   if ( syntax == nil ) then return end

   -- If that worked, and there are keywords..
   if (syntax and syntax['keywords'] ) then

      -- Set the keywords
      set_syntax_keywords( syntax['keywords'] )

      -- If there are defined values for the comments, set them too.
      if ( syntax['single'] and
           syntax['multi_open'] and
           syntax['multi_close'] ) then
         set_syntax_comments(syntax['single'],syntax['multi_open'], syntax['multi_close'] )
      end
   end
end


--
-- Run a command and return the output, if any.
--
function cmd_output(cmd)
   local handle = io.popen(cmd)
   local out = handle:read("*a")
   handle:close()
   return(out)
end




--
--  Copy & Paste functionality.
-----------------------------------------------------------------------------



--
-- Set the mark.  The region between the point & mark can be
-- cut easily.
--
function set_mark()
   x,y = point()
   mark( x,y)

   m_x, m_y = mark()
   status( "Mark set to " .. m_x .. "," .. m_y  )
end



--
-- This is the buffer which holds text which has been removed via:
--
--   Ctrl-y to delete the current line.
--
-- or
--
--   Ctrl-w to delete the region between the cursor and the mark.
--   (The mark being set by ctrl-space).
--
cut_buffer = ""


--
-- Kill a line - by copying it to our kill-buffer, and deleting the line
--
function kill_line()
   -- move to beginning of line
   sol()
   -- get the line, and save it
   cut_buffer = get_line() .. "\n"
   -- kill the line
   kill()
end

--
-- Insert the contents of the cut-buffer.
--
function yank()
   insert(cut_buffer)
end


--
-- Kill the region between the point and the mark.
--
-- Append the text between the point&mark to the cut-buffer.
--
function kill_between_point_and_mark()
   -- Save the selection
   cut_buffer = selection()

   -- Nuke it
   cut_selection()
end






--
--  Quit handling.
-----------------------------------------------------------------------------



--
-- Kill the current buffer
--
function confirm_kill_buffer()
   if ( not dirty() ) then
      kill_buffer()
      return
   end

   -- Buffer is dirty.
   status( "Buffer is dirty.  Really kill (y/n)?" )

   while( true ) do
      k = key()
      if ( k == "y" or k == "Y" ) then
         kill_buffer()
         status("Killed")
         return
      end
      if ( k == "n" or k == "N" ) then
         status("Cancelled")
         return
      end
   end
end


--
-- How many times has Ctrl-q been pressed?
--
quit_count = 2

--
-- Called on quit.
--
--  * If the buffer is clean then exit immediately.
--  * If the buffer is dirty require N Ctrl-q presses to exit.
--     * Although this is odd to me, this is how kilua worked in pure C.
--
function quit()
   --
   -- Count of dirty-buffers.
   --
   local dirty_count = 0

   --
   -- The current-buffer
   --
   local old_buffer = buffer()
   --
   -- For each buffer
   --
   for i=0,(buffers()-1) do
      --
      -- Select the buffer
      --
      buffer( i )

      --
      -- Is it dirty?
      --
      if ( dirty() ) then
         dirty_count = dirty_count + 1
      end
   end

   --
   -- Restore the old-current buffer
   --
   buffer( old_buffer )

   --
   -- Show the dirty/total buffers.
   --
   if ( dirty_count > 0 ) then
      if ( quit_count > 0 ) then
         status( dirty_count .. "/" .. buffers() .. " buffers are dirty, repeat " .. quit_count .. " more times to exit!")
         quit_count = quit_count - 1
      else
         exit()
      end
   else
      exit()
   end
end


--
--  Interactive evaluation
-----------------------------------------------------------------------------

function eval_lua()
   local txt = prompt( "Eval:" )
   if ( txt ) then
      local err, result = loadstring( txt )()
      if ( err ) then
         status( result )
      end
   else
      status( "Evaluation cancelled" )
   end
end


--
--  Insert the contents of a file/command at the point.
-----------------------------------------------------------------------------

--
-- NOTE: M-! will also insert the contents of a commands' output.
--
function insert_contents()

   local file = prompt( "Insert? " )

   if ( file == nil  or file == "" ) then
      status( "Cancelled" )
      return
   end

   --
   -- Is this a command?
   --
   if ( file:sub( 1, 1 ) == "|" ) then
      insert( cmd_output( file:sub(2) ) )
      return
   else
      -- File
      for line in io.lines(file) do
         insert( line )
         insert( "\n")
      end
   end
end

--
--  Implementation of setting/jumping to (named) marks.
-----------------------------------------------------------------------------


--
-- Swap the position of the point and mark
--
function swap_point_mark()
   m_x, m_y = mark()

   if ( m_x == -1 and m_y == -1 ) then
      -- no mark
      status( "No mark is set!" )
      return
   end

   x,y = point()
   mark( x, y)
   point( m_x , m_y )
end


--
-- Read a character, then use that to set a mark.
--
-- Marks are bound to `M-m XX` where XX is the key
-- the user entered.
--
function record_mark()
   k = expand_key(key())

   -- Ensure the values persist
   local x
   local y
   x,y = point()

   keymap['M-m'][k] = function() point(x,y) end
   status( "M-m " .. k .. " will now take you to " .. x .. "," .. y )
end

--
-- Test function just to prove `on_idle` is called every second, or so.
--
function on_idle()
   --
   -- status( os.date() )
   --
end


--
-- Call `make` - showing the output in our `*MAKE*` buffer.
--
function make()
   --
   -- Select the buffer.
   --
   local result = select_buffer( "*Make*" )

   --
   -- If selecting by name failed then the buffer can't exist.
   --
   -- Create it.
   --
   if ( result == false ) then
      create_buffer( "*Make*" )
   end

   -- Ensure we append output
   eof()

   -- Run the command.
   insert(cmd_output( "make" ) )

   -- completed
   insert("completed\n")
end


--
-- String interopolation function, taken from the Lua wiki:
--
--   http://lua-users.org/wiki/StringInterpolation
--
-- Usage:
--
--   print( string.interp( "Hello ${name}", { name = "World" } )
--
function string.interp(s, tab)
   return (s:gsub('($%b{})', function(w) return tab[w:sub(3, -2)] or w end))
end


--
-- Move to the next buffer
--
function next_buffer()
   local max = buffers()
   local cur = buffer()
   if ( cur < max ) then
      buffer( cur + 1)
   end
end

--
-- Move to the previous buffer.
--
function prev_buffer()
   local cur = buffer()
   if ( cur > 0 ) then
      buffer( cur - 1)
   end
end





--
-- If this function is defined it will be invoked to draw the status-bar.
--
-- Comment it out, or remove it, to fall-back to the C-based implementation.
--
function get_status_bar()
   --
   -- Format String of what we show.
   --
   local fmt = "${buffer}/${buffers} - ${file} ${modified} #BLANK# Col:${x} Row:${y} [${point}] ${time}"


   --
   -- Things we use.
   --
   local x,y = point()

   --
   -- Table holding values we can interpolate.
   --
   local t = {}
   t['buffer']          = buffer() + 1
   t['buffers']         = buffers()

   --
   -- See https://www.lua.org/pil/22.1.html
   --
   t['date']            = os.date("%A %d %B %Y")
   t['time']            = os.date("%X")

   t['file']            = buffer_name()
   t['x']               = x
   t['y']               = y + 1

   t['point'] = at()

   if ( dirty() )  then
      t['modified'] = "<modified>"
   else
      t['modified'] = ""
   end

   --
   -- Width of console
   --
   local w = width()

   --
   -- Interpolate our output
   --
   local out = string.interp( fmt, t )

   --
   -- Too large?
   --
   if ( #out > w ) then
      return( out:sub(0,w) )
   end

   --
   -- Too small?
   --
   local pad = w - #out + ( 7 )   -- 7 == length of '#BLANK#'
   local spc = ""
   while( pad > 0 ) do
      spc = spc .. " "
      pad = pad - 1
   end

   --
   -- Replace the #BLANK# string with our new spc-string containing the
   -- correct amount of padding.
   --
   out = out:gsub( "#BLANK#", spc )
   return( out )
end


function goto_line(number)
   if ( number == nil ) then
      number = prompt( "Goto line?" )
   end

   if tonumber(number) == nil then
      status( "You must enter a number!" )
      return
   end

   number = tonumber(number)

   --
   -- Move to start of file
   --
   sof();


   while( number >0 ) do
      move( "down" );
      number = number - 1;
   end
end
