--[[

 This is the configuration file for kilua.

 When kilua starts it will look for its configuration-file in two
 locations:

   ~/.kilua.lua
   ./kilua.lua

 If neither location exists this is a fatal error.

 There are three callback functions that kilua invokes at various times:

  * on_key(key)
     Called when input is received.

  * on_loaded(filename)
     Called after a file is loaded.

  * on_saved(filename)
     Called after a file is saved.

 Otherwise the only magic here is the `keymap` table.  Kilua will lookup
 every keypress in this table, and if there is a matching function defined
 it will be invoked, otherwise the literal character will be inserted.

Steve
--
https://steve.kemp.fi/


--]]


--
-- Keymap of bound keys
--
keymap = {}

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
keymap['^ ']        = function() set_mark() end
keymap['^A']        = sol
keymap['^D']        = function() insert( os.date() ) end
keymap['^E']        = eol
keymap['^F']        = find
keymap['^H']        = delete
keymap['^_']        = undo
keymap['^G']        = function() search( prompt( "Search:" ) ) end
keymap['^J']        = function() goto_mark() end
keymap['^K']        = function() kill_line() end
keymap['^L']        = eval
keymap['^M']        = function() insert("\n" ) end
keymap['^N']        = function() record_mark() end
keymap['^Q']        = function() quit() end
keymap['^O']        = open
keymap['^R']        = function() cmd = prompt( "execute:" ); if ( cmd ) then insert( cmd_output(cmd) ) end end
keymap['^S']        = save
keymap['^T']        = function() status( os.date() ) end
keymap['^U']        = function() yank() end
keymap['^W']        = function() kill_between_point_and_mark() end
keymap['^Z']        = undo
keymap['BACKSPACE'] = delete
keymap['DEL']       = delete
keymap['PAGE_UP']   = page_up
keymap['PAGE_DOWN'] = page_down
keymap['LEFT']      = left
keymap['RIGHT']     = right
keymap['UP']        = up
keymap['DOWN']      = down
keymap['HOME']      = function() point(0,0) end
keymap['END']       = function() end_of_file() end

--
-- M-x -> eval, just like emacs.
--
keymap['M-x']       = eval

--
-- Meta-Space record mark.
--   When you record a mark you'll be prompted for a key, and that
-- will be saved as "M-m X".
--
keymap['M- ']       = function() record_mark() end
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
keymap['^X']['^X'] = function() swap_point_mark() end


--
-- Syntax highlighting
--
local syn = {}
syn['c']   = {
   keywords = {
      -- These come first because otherwise "if" matches the token "#if".
      "#endif|", "#if|", "#ifdef|", "#ifndef|", "#include|",
      -- This is duplicated to cover both regexp support enabled & disabled.
      "#define|","^#\\s*define|",
      -- Keywords
      "auto","break","case","const","continue","default","do","else","enum","extern", "enum","for","goto","if","register","return","sizeof","switch","while",
      -- Types
      "char|","double|","extern|","float|","int|","long|","short|","signed|","static|","struct|","typedef|","union|","unsigned|","void|","volatile|",
   },
   single      = "//",
   multi_open  = "/*",
   multi_close = "*/"
}
syn['c++'] = syn['c']
syn['cc']  = syn['c']
syn['h']  = syn['c']

syn['lua'] = { keywords =
               { "and", "break", "do", "else", "elseif", "end", "false",
                 "for", "function", "if", "in", "local", "nil", "not",
                 "or", "repeat", "return", "then", "true", "until",
                 "while" },
               single      = "--",
               multi_open  = "--[[",
               multi_close = "--]]"
}
syn['pl'] = { keywords =
              { "continue", "foreach", "require", "package", "scalar", "format", "unless", "local", "until", "while", "elsif", "next", "last", "goto", "else", "redo", "our", "sub", "for", "use", "no", "if",  "my" },
              single      = "# ",
              multi_open  = "",
              multi_close = ""
}

syn['sh'] = { keywords =
              {
                 "case", "do", "done", "else", "env", "esac", "exit","export","fi","for","function","getopts","hash","if","import","in","let","local","read","select","set","shift","source","then","trap","true","type", "until", "while",
              },
                            single      = "# ",
                            multi_open  = "",
                            multi_close = ""
}




--
--  Callbacks and utilities they use.
-----------------------------------------------------------------------------

--
-- Expand the given keyboard character.
--    e.g. "ctrl-a" -> "^A".
--
function expand_key(k)

   -- Convert to decimal
   local b = string.byte(k)

   if ( b == nil ) then return "^ " end

   if ( b == 27 ) then
      return "ESC"
   end

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
do
   pending_char = nil
   pending_esc  = false

   function on_key(k)

      --
      -- Expand the character we've received to deal with
      -- control-codes, & etc.
      --
      k = expand_key(k)

      --
      -- Pending-escape?
      --
      if ( pending_esc == true ) then
         k = "M-" .. k
         pending_esc = false
      end


      --
      -- Was this escape?
      --
      if ( k == "ESC" ) then
         pending_esc = true
         return
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
            pending_input = {}
            return
         end
         if ( type(result) == 'table' ) then
            --
            -- This is a pending multi-part key - record this part
            -- away.
            --
            pending_char = k
            return
         end
      end

      --
      --
      -- Otherwise just insert the character.
      --
      insert(k)
      pending_char = nil
      --
      -- If we reached here the previous character was not Ctrl-q
      -- so we reset the global-quit-count
      --
      quit_count = 2
   end
end


--
-- This function is called when a file is loaded, and is used
-- to setup the syntax highlighting.
--
function on_loaded( filename )

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
-- Move to end of file
--
function end_of_file()
   -- Move to top of file.
   point(0,0)

   -- Move to last line
   repeat
      local x,y = point()
      down()
      local a,b  = point()
   until ( a==x ) and ( b == y )

   -- Move to end of line.
   eol()

end


--
--  Copy & Paste functionality.
-----------------------------------------------------------------------------



--
-- Set the mark.  The region between the point & mark can be
-- cut easily.
--
function set_mark()
   x,y = point();
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
-- Simple function to do a global search&replace.
--
-- Invoke via "Ctrl-l" then "search_replace('kemp','smith')"
--
-- NOTE: Bad things will happen if the replacement text also matches
--       the search pattern.
--
function search_replace( orig, new )
   local length = search( orig )

   while( length > 0 ) do
      -- Delete the match
      for i=1,length  do
         right()
         delete()
      end
      -- Insert the replacement
      insert( new )

      -- Repeat the search.
      length = search( orig )
   end
end
