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
--  * on_syntax_highlight(txt)
--     Called to perform the syntax highlighting of the current buffer.
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
--  You'll see some bindings are of the form "function() ... end", this is
-- required to cope with the fact that the function they invoke is not yet
-- defined, as it comes later in this file.
--
--  For example this would fail:
--
--     keymap['^D'] = delete_forwards
--
--  But by the time this file is _completely_ loaded the function is
-- indeed defined which allows this to work:
--
--     keymap['^D'] = function() delete_forwards() end
--
--
keymap['ENTER']         = function() insert("\n") end
keymap['KEY_BACKSPACE'] = delete
keymap['^H']            = delete
keymap['^D']            = function() delete_forwards() end

--
-- Ctrl-s is search
--
keymap['^S'] = function()
   local term = prompt("(regexp) Search? " )
   if ( term ) then
      search( term )
   end
end

keymap['M-g' ] = function() goto_line() end

--
-- TODO: keymap['^_'] = undo
-- TODO: keymap['^J'] = function() goto_mark() end
-- keymap['^K'] = function() kill_line() end
-- TODO: keymap['^N'] = function() record_mark() end
-- TODO: keymap['^U'] = function() yank() end
-- TODO: keymap['^Z'] = undo
--

--
-- Esc-q quits immediately.
--
keymap['M-q'] = exit

--
-- Movement
--
keymap['^A']         = sol  -- start of line
keymap['^E']         = eol  -- end of line
keymap['^B']         = function() move("left") end
keymap['KEY_LEFT']   = function() move("left") end
keymap['^F']         = function() move("right") end
keymap['KEY_RIGHT']  = function() move("right") end
keymap['^P']         = function() move("up") end
keymap['KEY_UP']     = function() move("up") end
keymap['KEY_DOWN']   = function() move("down") end
keymap['^N']         = function() move("down") end
keymap['KEY_HOME']   = sol   -- start of line
keymap['KEY_END']    = eol   -- end of line
keymap['M-KEY_HOME'] = sof   -- start of file
keymap['M-KEY_END']  = eof   -- end of file
keymap['KEY_PPAGE']  = function() page_up() end
keymap['KEY_NPAGE']  = function() page_down() end

--
-- Mark-Functions / Copy / Paste
--
keymap['^ ']  = function() toggle_mark() end
keymap['M-w'] = function() copy_selection() end
keymap['^W']  = function() cut_selection() end
keymap['^Y']  = function() paste() end

--
-- Non-ASCII testing functions:
--
keymap['KEY_IC'] = function() insert( "π" ) end
keymap['KEY_DC'] = function() insert( "€" ) end
keymap['€']      = function() status("Euros are like pounds, but smaller!") end
keymap['π']      = function() status("Mmmm, pie ..") end

--
-- M-x -> eval, just like emacs.
--
keymap['M-x'] = function() eval_lua() end

--
-- M-! ("Escape", then "!") will run a command and insert the output
-- into your document
--
keymap['M-!'] = function()
   local cmd = prompt( "execute:" )
   if ( cmd ) then
      insert( cmd_output(cmd) )
   end
end


--
-- M-m will record a bookmark.
--
keymap['M-m'] = function() record_mark() end

--
-- Prefix-map for jumping to recorded bookmark.
--
keymap['M-b'] = {}


--
-- Prefixed keybindings
--
--  ^X ^S => Save
--  ^X ^C => Exit
--  ^X  i => Insert file/command
--  ^X ^X => Swap point and mark
--
keymap['^X'] = {}
keymap['^X']['^C'] = function() quit() end
keymap['^X']['^S'] = save
keymap['^X']['^O'] = function() open_file() end
keymap['^X']['i']  = function() insert_contents() end

--
-- Working with buffers.
--
--
keymap['M-KEY_LEFT']   = function() prev_buffer() end
keymap['M-KEY_RIGHT']  = function() next_buffer() end
keymap['^X']['B']  = choose_buffer
keymap['^X']['K']  = kill_buffer
keymap['^X']['b']  = choose_buffer
keymap['^X']['c']  = create_buffer
keymap['^X']['k']  = function() confirm_kill_buffer() end
keymap['^X']['n']  = function () next_buffer() end
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
--
function on_loaded( filename )

   if ( not filename ) then return end

   -- Get the file-name + suffix.
   local file = filename:match("^.+/(.+)$") or filename
   local ext = file:match("^.+%.(.+)$") or file

   --
   --  Association for suffix to mode.
   --
   local x  = {}
   x['c']   = "cc"
   x['cc']  = "cc"
   x['h']   = "cc"
   x['cpp'] = "cc"
   x['lua'] = "lua"
   x['ini'] = "ini"
   x['md']  = "markdown"
   x['txt'] = "markdown"
   x['el']  = "lisp"

   --
   -- Setup syntax. Hack.
   --
   if ( x[ext] ) then
      syntax( x[ext] )
      status( "Selected syntax-mode " .. x[ext])
      return
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
--  Core things
--
-----------------------------------------------------------------------------


--
-- Prompt for a name and open it.
--
function open_file()

   --
   -- Is there a currently dirty file?
   --
   if ( dirty() ) then

      -- Buffer is dirty.
      status( "Buffer is dirty.  Really replace? (y/n)?" )

      local run = true

      while( run == true ) do
         k = key()
         if ( k == "y" or k == "Y" ) then
            run = false
         end
         if ( k == "n" or k == "N" ) then
            status("Cancelled")
            return
         end
      end
   end

   --
   -- Either no dirty-buffer, or the user didn't care
   --
   local name = prompt( "Open:" )
   if ( name and name ~= "" ) then
      open(name)
   end
end




--
--  Utility functions.
--
-----------------------------------------------------------------------------

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
-- Delete forwards
--
function delete_forwards()
   local x,y = point();
   move("right")
   local nx,ny = point()
   if ( nx == x and   ny == y  ) then
      return
   else
      delete()
   end
end


--
-- Call `make` - showing the output in our `*MAKE*` buffer.
--
function make()
   --
   -- Select the buffer.
   --
   local result = buffer( "*Make*" )

   --
   -- If selecting by name failed then the buffer can't exist.
   --
   -- Create it.
   --
   if ( result == -1 ) then
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
   for index,name in ipairs(buffers()) do
      --
      -- Select the buffer
      --
      buffer( index - 1 )

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
         status( dirty_count .. "/" .. #buffers() .. " buffers are dirty, repeat " .. quit_count .. " more times to exit!")
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
--  Functions relating to buffers.
--
-----------------------------------------------------------------------------


--
-- Move to the next buffer
--
function next_buffer()
   local max = #buffers()
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
--  Implementation of setting/jumping to (named) marks.
-----------------------------------------------------------------------------


--
-- Read a character, then use that to set a mark.
--
-- Marks are bound to `M-m XX` where XX is the key
-- the user entered.
--
function record_mark()
   status( "Press key to use for bookmark")
   k = key()

   -- Ensure the values persist
   local x
   local y
   x,y = point()

   keymap['M-b'][k] = function() point(x,y) end
   status( "M-b-" .. k .. " will now take you to " .. x .. "," .. y )
end







--
--  Callback functions
--
-----------------------------------------------------------------------------


--
-- `on_idle` is called every second, or so, and can run things in the
-- background.
--
function on_idle()
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
-- If this function is defined it will be invoked to draw the status-bar.
--
-- We use a format-string to determine what to draw, and ensure that we
-- pad/truncate to fit the width of the console
--
function get_status_bar()

   --
   -- Format String of what we show.
   --
   local fmt = "${buffer}/${buffers} - ${file} ${mode} ${modified} #BLANK# Col:${x} Row:${y} [${point}] ${time}"

   --
   -- Things we use.
   --
   local x,y = point()

   --
   -- Table holding values we can interpolate.
   --
   local t = {}
   t['buffer']  = buffer() + 1
   t['buffers'] = #buffers()

   --
   -- See https://www.lua.org/pil/22.1.html
   --
   t['date'] = os.date("%A %d %B %Y")
   t['time'] = os.date("%X")

   t['file']  = buffer_name()

   t['mode']  = ""

   local s = syntax()
   if ( #s > 0 ) then
      t['mode'] = s .. "-mode"
   end
   t['x']     = x
   t['y']     = y + 1
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
   -- If the format string includes `${point}` - which expands to the
   -- character under the cursor - then we need to make sure that is
   -- only a single character.
   --
   -- If the character under the point is `€`, or other multi-byte
   -- character, it will have a length of bigger than one byte
   -- even though it is, by definition, a single character.
   --
   local at_pos = string.find( fmt, "${point}" )
   local fix = 0
   if ( at_pos ) then
      fix = #at() -1
   end

   --
   -- Too large?
   --
   if ( #out > w ) then
      return( out:sub(0,w) )
   end

   --
   -- Too small?
   --
   local pad = w - #out + fix + ( 7 )   -- 7 == length of '#BLANK#'
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



--
-- Callback function that handles parsing
--
function on_syntax_highlight( text )
   --
   -- Get the syntax mode
   --
   local mode = syntax()
   if ( not mode ) then
      return ""
   end

   --
   -- Load LPEG
   --
   lpeg = load_syntax( 'lpeg' )
   if ( not lpeg ) then
      status("Lua LPEG library not available - syntax highlighting disabled")
      syntax("")
      return
   end



   --
   -- Load the module, which will be cached the second
   -- time around.
   --
   local obj = load_syntax( mode )
   if ( obj ) then
      return(tostring(obj.parse(text)))
   else
      status("Failed to load syntax-module '" .. syntax() .. "' disabling highlighting.")
      syntax("")
      return("")
   end
end



--
--  Functions relating to movement.
--
-----------------------------------------------------------------------------


--
-- Move up a screenful of text.
--
function page_up()
   local h = height() - 1
   while( h > 0 ) do
      move( "up" )
      h = h - 1
   end
end

--
-- Move down a screenful of text.
--
-- Leave two lines of context.
--
function page_down()
   local h = height() - 1
   while( h > 0 ) do
      move( "down" )
      h = h - 1
   end
end


--
-- Move to the given line-number.
--
-- NOTE: This is naive because it doesn't test that you entered a line-number
-- that doesn't exist.
--
-- That said it does work, and in the worst case it can't do a bad thing.
--
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


--
--  Syntax Highlighting
--
--
-----------------------------------------------------------------------------

--
-- Load syntax files from:
--
--   /etc/kilua/syntax/
--   ~/.kilua/syntax/
--
package.path = package.path .. ';/etc/kilua/syntax/?.lua'
package.path = package.path .. ';' .. os.getenv("HOME") .. '/.kilua/syntax/?.lua'


function load_syntax( lang )

   -- Save the original path
   orig = package.path

   -- Add the per-run one.
   if ( syntax_path ) then
      package.path = package.path .. ';' .. syntax_path .. "/?.lua"
   end

   -- Load the library
   local status, obj = pcall(require, lang)

   -- Restore the path
   package.path = orig

   if ( status ) then
      -- enable syntax highlighting.
      return obj
   else
      -- disable syntax highlighting.
      return nil
   end
end

--
-- Show the version
--
status("Kilua v" .. KILUA_VERSION)


--
-- Dump details about our buffers
--
function bd()
   --
   -- Create a buffer to show our output
   --
   local result = buffer( "*Buffers*" )
   if ( result == -1 ) then
      create_buffer( "*Buffers*" )
   end

   --
   -- Move to the end of the file.
   --
   eof()

   --
   -- Get the buffers
   --
   local b = buffers()
   insert( "There are " ..  #b .. " buffers" .. "\n" )

   --
   -- Show them.
   --
   for i,n in ipairs(b) do
      insert( "Buffer " .. i .. " has name " .. n .. "\n" )
   end

end
