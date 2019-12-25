# pager - A Terminal-Pager
A small utility similar to *less*.

## Usage
```pager.exe <filename>``` \
    or \
```other command | pager.exe```

Controls:

    j, ENTER, ARROW_DOWN    = scroll 1 line down
    k, ARROW_UP             = scroll 1 line up
    SPACE, PG_DOWN          = scroll 1 page down
    b, PG_UP                = scroll 1 page up
    g, HOME                 = jump to the beginning
    G, END                  = jump to the end
    v                       = show file in editor (exit pager)
    ?                       = help
    q                       = exit

If a filename is given, it will be read. Otherwise pager.exe trys to read from 'stdin'.

## Known Bugs/Missing Features
- lines longer than the with of the terminal are cut off, no linewrapping or horizontal scrolling atm...
- no handling of terminal resizing, the application just exits if it detects resizing of the terminal.
- crashes if input < than terminal hight.