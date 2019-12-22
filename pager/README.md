# pager - A Terminal-Pager
A small utility similar to *less*.

## Usage
```pager.exe <filename>``` \
    or \
```other command | pager.exe```

Controls:

    ARROW_UP, K    = scroll 1 line up
    ARROW_DOWN, J  = scroll 1 line down
    PG_UP, l       = scroll 1 page up
    PG_DOWN, h     = scroll 1 page down
    HOME           = jump to beginning of the input
    END            = jump to end of the input
    ESC, q          = exit

If a filename is given, it will be read. Otherwise pager.exe trys to read from 'stdin'.

## todo ....