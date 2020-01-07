![](https://github.com/holgerdoerner/TermTools/workflows/CTest/badge.svg) ![](https://github.com/holgerdoerner/TermTools/workflows/Release%20Packaging/badge.svg) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/7f1fa4df8d47447daf76710556eed469)](https://www.codacy.com/manual/HolgerDoerner/TermTools?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=HolgerDoerner/TermTools&amp;utm_campaign=Badge_Grade)

[![GitHub release](https://img.shields.io/github/release/HolgerDoerner/TermTools.svg)](https://GitHub.com/HolgerDoerner/TermTools/releases/) [![GitHub license](https://img.shields.io/github/license/HolgerDoerner/TermTools.svg)](https://github.com/HolgerDoerner/TermTools/blob/master/LICENSE)
# TermTools <!-- omit in toc -->
A collection of tools for the terminal written in C. They are first and foremost aimed for the Windows platform.

- [About](#about)
- [Included Tools](#included-tools)
- [Pre-Compiled Binaries](#pre-compiled-binaries)
  - [Installing](#installing)
- [Build](#build)
  - [Requirements](#requirements)
  - [Getting startet on the command-line](#getting-startet-on-the-command-line)
    - [Configure Environment after installing](#configure-environment-after-installing)
- [FAQ](#faq)
  - [Why in the hell C ?](#why-in-the-hell-c)
  - [Why not Java or better, C#? Especialy on Windows?](#why-not-java-or-better-c-especialy-on-windows)

# About
This collection of tools for the Windows Command-Line is a small side-/hobby-project i wanted to do for quite a long time. I always felt sad about some funktionality which we can take for given when working on the terminal in other OS-es but are missing on Windows. Yes I know there is PowerShell and WSL and those are great, but I love to write native applications, especialy in C. Also, those tools work regardless of the used shell (cmd.exe, PowerShell).

Also those tools are not meant to be clones or re-implementations of other well-known applications, mostly from the Linux-World, but they are of course highly inspired by them. The implementations are as simple as possible, no fancy stuff if not ABSOLUTLY needed. Just no-nonsense get-the-job-done stuff...

***Further Notes:*** I target the more recent versions of Windows 10 and Windows Server, and I don't test on older ones. So, if You really need dem to run on Windows XP or similar and they dont't, open a ticket and I will see what I can do for You.

# Included Tools
NAME | DESCRIPTION
--- | ---
pager | A terminal pager. More like less...
counter | Simple counter. Counts lines or words in text-files.
winfetch | Displays System-Information on the terminal.
timer | Measures duration of commands.
hashsum | Calculates Hash-Digests.

# Pre-Compiled Binaries
The provided pre-compiled binaries on the releases-page are all 64-bit, if not mentoned otherwise. If You really need 32-bit binaries, You have to compile them yourself (*see:* [Build](#build)).

## Installing
To use them, just extract the ZIP-archive to a location of Your choice (suggestion: `%USERPROFILE%\TermTools`) and update Your Environment-Settings (*see:* [Configure Environment after installing](#configure-environment-after-installing)).

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

To get a list of available targets, type

    C:\TermTools\build> nmake help

And to install the binaries do a

    C:\TermTools\build> nmake install

*Note:* Install needs a 'Release'-configuration!

The default install_prefix is: `%USERPROFILE%\TermTools`.

### Configure Environment after installing
It makes sense to be abele to call the tools anytime from a terminal. To do this You have to set a new Environment-Variable `TERMTOOLS_HOME=%USERPROFILE%\TermTools` and append `TERMTOOLS_HOME` to Your User-`Path`.

# FAQ
## Why in the hell *C* ?
Because I like it, that's why!! I started programming back in the mid 90's lerning C on my own. Besides of that, it is an **AWESOME** language, and doesn't get the respect nowdays it realy deserves! It is a fast, get shit done language with no-nonsense (okay, almost).

## Why not Java or better, C#? Especialy on Windows?
I could have done that, but I have to work with this 'high-level' stuff day-to-day as an full-time developer. I do *TermTools* in my free time (so as the other stuff an my  GitHub account), and in **MY** free time I like to use what's the most exciting and fun to me. Or stuff that teaches me new things. On the other hand, the most technologies today bring in *A LOT* of overhead, definetly too much for small applications like this.