# timer - Measures duration
A small utility to measure and display the duration of commands.

It shows the number of Ticks elapsed and breaks them down to hours, minutes, seconds, milliseconds and microseconds.

## Usage
`timer.exe [/?] <COMMAND>`

Arguments:

    COMMAND       - The command to measure
    /?            - Print help

If the command takes Arguments by itself, it's call has to be quoted eg: `timer.exe "DIR /S C:\Windows\System32"`.

## Known Bugs/Missing Features
- problems with programs not writing any output to STDOUT, timer than hangs forever.