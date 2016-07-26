
Editor
------

Proof of concept editor which handles UTF-8 and has Lua support.


Usage
-----

Once built you should find it works as you would expect.

To move around use the arrows, to change the keybindings look at `keymap`,
and to execute Lua you should use `M-x` - which will open a prompt for you
to enter into.

You'll find a lot of the functionality is designed around the prompt,
for example you can execute:

     M-x goto_line()
     M-x goto_line(3)
     M-x save( "/tmp/test.txt" )
     M-x open( "/etc/motd" )


Design and Implementation
-------------------------

Almost directly cribbed from `kilo`:

* Editor stores buffers.
    * Buffers contain
       - Rows
       - Cursor-magic.
       - A name.
       - A dirty-flag

The editor is built around the `Editor` class, implemented in `editor.cc` & `editor.h`.  The Editor object is a singleton, because the (static) Lua functions need to be able to gain access to the instance to manipulate the open buffers, & etc.

The Lua implementation is divided into a number of files by purpose:

* `lua_buffers.cc`
   * Functionality relating to buffers.
* `lua_core.cc`
   * Core functionliaty.
* `lua_movement.cc`
   * Functionality relating to movement.
* `lua_screen.cc`
   * Functionality relating to the screen.

The division might be somewhat arbitrary.
