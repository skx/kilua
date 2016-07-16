= Bugs =

* Highlighting should have a case-insensitive flag, for SQL.

* We are missing syntax-highlighting for Python, Ruby, Puppet, CFEngine, and similar.

* You can set a mark, and use that to cut the region between point and
  mark - but you cannot see the region!


= Possible Bugs =

Needs to be ran under valgrind to look for memory-leaks, almost
certainly we should free our syntax and row-structures in our
at_exit handler.
