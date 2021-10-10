![Brain Pain](https://raw.githubusercontent.com/kurtjd/brain-pain/master/brainpain.png)

Brain Pain
======
A command-line clone of the classic Simon game.

Controls
========
* Press keys 1-4 to select the corresponding quadrant.

Requirements
============
* Curses

Build Procedures
=================
Game is only a single file. Just link ncurses and compile, like so:

```gcc brainpain.c -lncurses -o brainpain```

If your compiler doesn't default to the C99 standard, make sure to explicity state it, like so:

```gcc -std=gnu99 brainpain.c -lncurses -o brainpain```

Run
===
```./brainpain```
