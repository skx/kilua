= Bugs =

* Highlighting should have a case-insensitive flag, for SQL.

* We are missing syntax-highlighting for Perl, Python, Ruby, Puppet,
  CFEngine, and similar.


= Possible Bugs =

Needs to be ran under valgrind to look for memory-leaks, almost
certainly we should free our syntax and row-structures in our
at_exit handler.
