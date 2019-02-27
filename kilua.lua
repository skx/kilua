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
-- embedded into the generated binary, as part of the build-process.
--
-- There are several functions that kilua invokes at various times, these
-- callbacks are documented later in the file, but in brief they include:
--
--  * get_status_bar()
--     This function is called to populate the status-bar in the footer.
--
--  * on_complete(str)
--     This is invoked if the user presses `TAB` when prompted for input.
--
--  * on_idle(key)
--     Called when things are idle, to allow actions to be carried out.
--
--  * on_loaded(filename)
--     Called when a file is loaded, and enables syntax highlighting.
--
--  * on_save(filename)
--      Called __before__ a file is saved.
--
--  * on_saved(filename)
--      Called __after__ a file is saved.
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
keymap['KEY_DC']        = function() delete_forwards() end
keymap['^H']            = delete
keymap['^D']            = function() delete_forwards() end


do
   --
   -- This is the value that the user entered previously, and is
   -- the default if nothing is entered again.
   --
   local search_term = ''

   keymap['^S'] = function()
      local term = prompt("(regexp) Search? " )

      -- If nothing entered default to the previous value
      if term == nil or term == "" then
         term = search_term
      end

      if ( term ) then

         -- get the point
         local x,y = point();

         -- if we matched
         if ( search( term ) ) then
            -- get the new point
            local x2,y2 = point()

            -- did they change?  If not we move one to the right
            -- and try again
            if ( x == x2 and y == y2 ) then
               move("right")
               search(term)
            end
         end

         -- save the search term
         search_term = term
      end
   end
end

--
-- Goto line
--
keymap['M-g' ] = function() goto_line() end

--
-- Cut-line, and paste-line.
--
keymap['^K'] = function() kill_line() end
keymap['^Y'] = function() paste() end

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
-- M-x -> eval, just like emacs.
--
keymap['M-x'] = function() eval_lua() end

--
-- M-! ("Escape", then "!") will run a command and insert the output
-- into the current buffer.
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
keymap['^X']['i']  = function() insert_contents() end

-- ^X ^O open file in new buffer
-- ^X ^V open file in existing buffer
keymap['^X']['^O'] = function() open_file(true) end
keymap['^X']['^V'] = function() open_file(false) end

--
-- Working with buffers.
--
keymap['M-KEY_LEFT']  = function() prev_buffer() end
keymap['M-KEY_RIGHT'] = function() next_buffer() end
keymap['^X']['B']     = function() choose_buffer() end
keymap['^X']['K']     = kill_buffer
keymap['^X']['b']     = function() choose_buffer() end
keymap['^X']['c']     = create_buffer
keymap['^X']['k']     = function() confirm_kill_buffer() end
keymap['^X']['n']     = function() next_buffer() end
keymap['^X']['p']     = function() prev_buffer() end


--
-- Working with the selection
--
keymap['^ ']  = function() toggle_mark() end
keymap['M-w'] = function() copy_selection() end
keymap['^W']  = function() cut_selection() end




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
-- There is some magic to handle multi-character keystrokes
-- but it is pretty simple.
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
   end
end


--
-- This function is called when a file is loaded, and is used
-- to setup the syntax highlighting.
--
-- You can replace this function if you don't like the defaults
-- or leave as-is if you do.
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
   x['c']        = "cc"
   x['cc']       = "cc"
   x['cpp']      = "cc"
   x['el']       = "lisp"
   x['go']       = "go"
   x['h']        = "cc"
   x['htm']      = "html"
   x['html']     = "html"
   x['email']    = "email"
   x['msg']      = "email"
   x['ini']      = "ini"
   x['lua']      = "lua"
   x['md']       = "markdown"
   x['txt']      = "markdown"
   x['Makefile'] = "makefile"

   --
   -- Setup syntax. Hack.
   --
   if ( x[ext] ) then
      syntax( x[ext] )
      status( "Selected syntax-mode " .. x[ext] .. " via suffix " .. ext )

      -- Trigger immediate re-redender.
      on_idle()

      return
   end
   if ( x[file] ) then
      syntax( x[file] )
      status( "Selected syntax-mode " .. x[ext] .. " via filename " .. file )

      -- Trigger immediate re-redender.
      on_idle()

      return
   end
end



--
-- This function is called BEFORE a file is saved.
--
-- You might strip trailing whitespace, indent, or similar.
--
function on_save( filename )
   status("About to save ..")
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
function open_file(new_buff)

   --
   -- If we're opening in a new buffer prompt for file
   -- then open it
   --
   if new_buff then
      local name = prompt( "Open:" )
      if ( name and name ~= "" ) then
         create_buffer()
         open(name)
      else
         status( "Cancelled" )
      end
      return
   end

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
   local name = prompt( "Replace with:" )
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
--  Handling for cutting the current line, and copying/cutting the
-- current selection.
--
-----------------------------------------------------------------------------


--
-- The paste-buffer
--
paste_buffer = ""

--
-- Kill the current line.
--
function kill_line()

   -- Move to the end of the line, so we can see how long it is
   eol()
   local x,y = point()
   sol()

   paste_buffer = ""

   -- The count of characters we must remove
   local count = x

   -- While we've not deleted all the characters, do so.
   while( count > 0 ) do

      -- Store the character under the point in the paste-buffer
      paste_buffer = paste_buffer .. at()

      -- Then delete it.
      delete_forwards()

      -- Bump the count.
      count = count - 1
   end
   paste_buffer = paste_buffer .. "\n"
end


--
-- Paste in our paste-buffer.
--
-- This is currently set by `Ctrl-k` which kills the current line.
--
function paste()
   insert( paste_buffer )
end


--
-- Set the mark, or remove the mark.
--
function toggle_mark()
   local mx, my = mark()
   if ( mx == -1 and my == -1 ) then
      -- set the mark
      local x,y = point()
      mark(x,y)
      status("Set mark to " .. x .. "," .. y )
   else
      -- clear the mark
      mark(-1,-1)
      status("Cleared mark")
   end
end


--
-- Copy the selection into the paste-buffer.
--
function copy_selection()
   local mx, my = mark()
   if ( mx == -1 and my == -1 ) then
      status("No selection!")
      return
   end

   paste_buffer = selection()
   mark(-1,-1)
end


--
-- Copy the selection into the paste-buffer, then delete it
--
function cut_selection()
   local mx, my = mark()
   local px, py = point()
   if ( mx == -1 and my == -1 ) then
      status("No selection!")
      return
   end

   -- save the selected text.
   paste_buffer = selection()

   -- now we want to move the the "end" of the selection
   if ( py > my or (px > mx and py == my)) then
      -- OK
   else
      point( mx, my )
   end
   move("right")

   -- delete
   len = #paste_buffer
   while( len > 0 ) do
      delete()
      len = len -1
   end

   -- remove the mark
   mark(-1,-1)
end








--
--  Quit handling.
-----------------------------------------------------------------------------



--
-- Kill the current buffer, prompting for confirmation if it is modified.
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
   -- If there are dirty buffers ..
   --
   if ( dirty_count > 0 ) then

      --
      -- Show the user how many buffers are dirty.
      --
      status( dirty_count .. "/" .. #buffers() .. " buffers are dirty.  Really quit?  (y/n)" )

      --
      -- Await confirmation
      --
      k = key()
      if ( k == 'y' ) or ( k == 'Y' ) then
         exit()
      else
         status("Cancelled!")
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
      status( "Evaluation cancelled!" )
   end
end


--
--  Insert the contents of a file at the point.
-----------------------------------------------------------------------------

function insert_contents()

   local file = prompt( "Insert? " )

   if ( file == nil  or file == "" ) then
      status( "Cancelled" )
      return
   end

   -- Open the file, and insert the contents.
   for line in io.lines(file) do
      insert( line )
      insert( "\n")
   end
end





--
--  Functions relating to buffers.
--
-----------------------------------------------------------------------------


--
-- Prompt for a buffer, interactively
--
function choose_buffer()

   --
   -- Save the current buffer
   --
   local cur = buffer()

   --
   -- Build up a table of each buffer-name
   --
   local m = {}

   --
   -- For each buffer.
   --
   for index,name in ipairs(buffers()) do

      -- Select it, so we can see if it is dirty
      buffer( index - 1 )

      -- The name and mode
      local name = buffer_name()
      local mode = syntax()
      if ( mode and mode ~= "" ) then
         mode = "[" .. mode .. "]"
      end

      -- Add the result to our menu-choice-table.
      if ( dirty() ) then
         table.insert(m, index .. " " .. name .. " <modified>" .. " " .. mode )
      else
         table.insert(m, index .. " " .. name .. " " .. mode  )
      end
   end

   --
   -- Make the menu-choice
   --
   local r = menu( m )

   --
   -- User cancelled.
   -- Restore the original buffer and return.
   --
   if ( r  == -1 ) then
      buffer( cur)
      return
   end

   --
   -- Otherwise change to the buffer the user chose.
   --
   buffer( r )
end


--
-- Move to the next buffer
--
function next_buffer()
   local max = #buffers()
   local cur = buffer()

   if ( (cur +1) < max ) then
      buffer(cur + 1)
   else
      buffer(0)
   end
end

--
-- Move to the previous buffer.
--
function prev_buffer()
   local cur = buffer()
   if ( cur > 0 ) then
      buffer( cur - 1)
   else
      local max = #buffers()
      buffer(max-1)
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
   local b
   x,y = point()
   b = buffer()

   keymap['M-b'][k] = function() buffer(b) point(x,y) end
   status( "M-b-" .. k .. " will now take you to " .. x .. "," .. y .. " in buffer " .. (b+1) )
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
   local text    = text()
   local colours = on_syntax_highlight( text );
   if ( colours ~= nil and colours ~= "" ) then
      update_colours( colours )
   end
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
-- Callback function to perform TAB-completion at the prompt.
--
-- This function should return the updated input to use at the prompt,
-- when the user presses TAB.
--
--    i.e. Input "/etc/pas", the result should be "/etc/passwd".
--
-- This only completes at the start of the string at the moment, but
-- extending it should be simple enough.
--
function on_complete( str )

   --
   -- Holder for things that we might be able to complete upon.
   --
   tmp = {}

   --
   -- Add in all user-defined functions.
   --
   for k,v in pairs(_G) do
      tmp[k] = k .. "("
   end

   --
   -- If the token starts with "~" then replace that with
   -- the users' home-directory
   --
   if ( string.sub( str, 0, 1 ) == "~" ) then
      str = string.sub( str, 2 )
      str = os.getenv("HOME") .. str
   end

   --
   -- Is the user attempting to complete on a file-path?
   --
   if ( string.match( str, "^/" ) ) then

      --
      -- Get the directory this is from.
      --
      -- Default to / if we found no match.
      --
      dir = string.match(str, "^(.*)/" )
      if ( dir == "" ) then dir = "/" end

      --
      -- If the directory exists then add all the entries to the completion-set.
      --
      if ( exists( dir ) ) then
         entries = directory_entries( dir )
         for i,v in ipairs(entries) do

            if ( exists( v .. "/." ) ) then
               tmp[v .. "/" ] = v .. "/"
            else
               tmp[v] = v
            end
         end
      end
   end

   --
   -- We have a list of things in `tmp` which _might_ match
   -- the user's input.
   --
   -- Now build up a table of the things that actually _do_ match
   -- what the user gave us.
   --
   ret = { }

   --
   -- Do we have a match?
   --
   for k,v in pairs(tmp) do
      if ( string.match( v, "^" .. str ) ) then
         table.insert(ret, v)
      end
   end

   --
   -- OK at this point we have a table `ret` with things that
   -- match the users's input.
   --


   --
   -- If there are zero entries then there is no completion.
   --
   if ( #ret == 0 ) then
      return( str )
   end

   --
   -- If there is just one entry - return it.
   --
   if ( #ret == 1 ) then
      return( ret[1] )
   end

   --
   -- Otherwise sort the choices and let the user choose.
   --
   table.sort(ret)

   --
   -- So we have ambiguity.  Resolve it
   --
   local res = menu( ret )

   --
   -- If the user cancelled that return the unmolested input
   --
   if ( res == -1 ) then
      return(str)
   else
      --
      -- Otherwise return what they chose.
      --
      return( ret[res+1] )
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
-- Callback function that handles syntax highlighting.
--
-- Given the text "Steve Kemp" this funciton should return
-- one character for each byte of that input.  The character
-- will be added to '0' and used to colour the input.
--
-- A defualt implementation would return
--
--       WHITE
--
-- For each character.
--
function on_syntax_highlight( text )
   --
   -- Get the syntax mode
   --
   local mode = syntax()
   if ( mode == nil or mode == "" ) then
      return ""
   end

   --
   -- Load LPEG - if that fails then unset the syntax for this
   -- buffer, which will ensure we're not called again in the future.
   --
   lpeg = load_syntax( 'lpeg' )
   if ( not lpeg ) then
      status("Lua LPEG library not available - syntax highlighting disabled")
      syntax("")
      return
   end



   --
   -- Load the module, which will be cached the second time around.
   --
   -- Again if this fails we'll disable syntax-highlighting for this
   -- mode, by unsetting `syntax`.
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
-- Show the version
--
status("Kilua v" .. KILUA_VERSION)





--
-----------------------------------------------------------------------------
--
-- Functions below here are samples and test-code by Steve
--
-- They might be inspirational, or they might kill your computer
--
-----------------------------------------------------------------------------
--


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


--
-- Dump files beneath /etc
--
function ls(path)

   --
   -- Create a buffer to show our output
   --
   local result = buffer( "*ls*" )
   if ( result == -1 ) then
      create_buffer( "*ls*" )
   end

   --
   -- Move to the end of the file.
   --
   eof()

   --
   -- Get the path
   --
   if ( path == nil ) or ( path == "" ) then
      path = "/etc"
   end

   --
   -- Get the directory entries
   --
   local b = directory_entries( path )
   insert( "There are " ..  #b .. " files" .. "\n" )

   --
   -- Show them.
   --
   for i,n in ipairs(b) do
      insert(  i .. " -> " .. n .. "\n" )
   end

end



function a()
   buffer_data("name", "steve")
end
function b()
   buffer_data("name", "Bob")
end
function c()
   insert(buffer_data("name"))
end
