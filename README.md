Simon
======
A command-line version of the classic Simon game.

Game is only a single file. Just link ncurses and compile, like so:

gcc nsimon.c -lncurses -o nsimon

If your compiler doesn't default to the C99 standard, make sure to explicity state it, like so:

gcc -std=gnu99 nsimon.c -lncurses -o nsimon
