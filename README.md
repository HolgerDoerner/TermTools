![](https://github.com/holgerdoerner/TermTools/workflows/CTest/badge.svg) ![](https://github.com/holgerdoerner/TermTools/workflows/Release%20Packaging/badge.svg)
# TermTools <!-- omit in toc -->
A collection of tools for the terminal written in C. They are first and foremost aimed for the Windows platform.

- [About](#about)
- [Included Tools](#included-tools)
- [Build](#build)
  - [Requirements](#requirements)
  - [Getting startet on the command-line](#getting-startet-on-the-command-line)
- [FAQ](#faq)
  - [Why in the hell C ?](#why-in-the-hell-c)
  - [Why not Java or better, C#? Especialy on Windows?](#why-not-java-or-better-c-especialy-on-windows)

# About

# Included Tools
NAME | DESCRIPTION
--- | ---
pager | A terminal pager. More like less...
counter | A simple counter. Counts lines or words in text-files.
winfetch | Displays System-Information on the terminal.

# Build
## Requirements
- **Visual Studio 2019** (16.2.4) or \
  **VC Build Tools 2019** (16.4.0)
- GIT (obvously...)

Lower versions (*should*) work, but are not testet.

The edition of Visual Studio is not important, any will work.

[PDCurses](https://github.com/wmcbrine/PDCurses) is included as a dependency inside of the *CMake* build-system. It get's cloned from GitHub automaticly when a build is started.

## Getting startet on the command-line
At first, clone the GitHub  repository:

    C:\> git clone https://github.com/holgerdoerner/TermTools.git

After that, start a *Visual Studio Developer Prompt* for your architecture with the batch-files of you *Visual Studio* installation, e.g:

`C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat` (for 64-bit)

or

`C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars86.bat` (for 32-bit)

In the newly opened terminal, change to the directory where you cloned *TermTools* into:

    C:\> cd <PATH_TO_LOCAL_TREE>\TermTools
    C:\TermTools> build_cmake.bat

Whitch generates a 'Debug' configuration. To use a 'Release' configuration, run

    C:\TermTools> build_cmake.bat Release

Now, when configuration finished without errors, you are in the *build* sub-directory of the source-tree. To compile just do a

    C:\TermTools\build> nmake
    ( or )
    C:\TermTools\build> nmake all

to compile all binaries, or pick one with

    C:\TermTools\build> nmake <TARGET_NAME>

to get a list of available targets, type

    C:\TermTools\build> nmake help

# FAQ
## Why in the hell *C* ?
Because I like it, that's why!! I started programming back in the mid 90's lerning C on my own. Besides of that, it is an **AWESOME** language, and doesn't get the respect nowdays it realy deserves! It is a fast, get shit done language with no-nonsense (okay, almost).

## Why not Java or better, C#? Especialy on Windows?
I could have done that, but I have to work with this 'high-level' stuff day-to-day as an full-time developer. I do *TermTools* in my free time (so as the other stuff an my  GitHub account), and in **MY** free time I like to use what's the most exciting and fun to me. Or stuff that teaches me new things. On the other hand, the most technologies today bring in *A LOT* of overhead, definetly too much for small applications like this.