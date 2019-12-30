# counter - A simple counter.
Counts lines or words in text-files.

## Usage
`counter.exe <filename>`

Switches:

    none        = count lines
    /W          = count words
    /?          = print help

Results are printed to `STDOUT` errors to `STDERR`.

Returns `0` on success and a value `!= 0` on error.