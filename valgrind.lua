--
-- Cause corruption.
--
function valgrind()
   -- Create an input-file
   os.execute( "cp kilo.c kilo.c.tmp" )

   -- Open it
   open( "kilo.c.tmp" )

   -- Move to end of file
   end_of_file()

   -- Delete a lot of lines.
   local c = 10000
   while( c > 1 ) do
      c = c -1
      delete();
   end

   -- All done
--   exit()
end
