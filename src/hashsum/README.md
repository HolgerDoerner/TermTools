# hashsum - Calculates Hash-Digests
Command-Line utility to calculate and verify hash-digests of one or more files.

Supportet Algorithems are:

    MD5
    SHA1
    SHA256
    SHA385
    SHA512

The (only) currently supported format for hash-files is:

    # line-comments are also supported
    FA47C8661BB2669342D1A541BFDE150D  hashsum.exe

While parsing a hash-file the application will try to determine the type of algorithem. A hash-file can also contain mixed types.

## Usage
Usage:
    
    HASHSUM.EXE [/MD5 /SHA1 /SHA256 /SHA384 /SHA512] <file> [files ...]
    HASHSUM.EXE [/C] <hash-file> [hash-files ...]

Options:

    /?          = shows usage info
    /C          = checks digests stored in hash-file(s)
    /MD5        = generate MD5-Digest
    /SHA1       = generate SHA1-Digest
    /SHA256     = generate SHA256-Digest (default)
    /SHA385     = generate SHA384-Digest
    /SHA512     = generate SHA512-Digest

## Known Bugs/Missing Features
- only one supported format for hash-files.