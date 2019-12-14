# TIM - Type IMproved
A small utility meant as a 'drop-in' replacement for 'type'-command.

## Usage
```TIM.EXE <filename>``` \
    or \
```other command | TIM.EXE```

Controls:

    ARROW_UP, K    = scroll 1 line up
    ARROW_DOWN, J  = scroll 1 line down
    PG_UP, l       = scroll 1 page up
    PG_DOWN, h     = scroll 1 page down
    HOME           = jump to beginning of the input
    END            = jump to end of the input
    ESC, CTRL+C    = exit

If a filename is given, it will be read. Otherwise TIM trys to read from 'stdin'.