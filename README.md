Introduction
============

My own pty wrapper. Intended for use in execline scripts. Mostly intended for
s6 services when launching a daemon that logs to stdout instead of stderror
and doesn't change its stdout to line-buffering.

Should work similar to DJB's getpty or ptyget or whatever it was called.

That utility was only licensed for unmodified redistribution, so I didn't look
at the source code.

This is intended to be GPL or BSD or something redistributable (I haven't
decided yet)

Building / Installation
=======================

This utility is very simple, requiring no libraries and hopefully no options to build.

Most users can just run `make ptywrap` to build the executable, then copy it
wherever you want.

Depending on your system, you may need to tell the compiler to use C99 instead
of C89. See your tool's documentation for instructions on how to do this.

Usage
=====

This program doesn't currently take any options. The first argument is the
name of a program to run, which may be by path or will be found by searching
the PATH. The remaining arguments are passed to the program directly.

Example 1

`ptywrap mydaemon | s6-log -t /var/log/mydaemon`

In the above example, the shell launches ptywrap and s6-log connected via a
pipe. Ptywrap runs mydaemon, redirecting it's tty output (both stdout and
stderr) to Ptywrap's output (which is the pipe to s6-log). S6-log then logs
entries linewise with good timestamps, even when mydaemon does not force its
stdout to linebuffered mode.

Note that ptywrap shouldn't require any special privileges to run. As such, it
should probably be the last thing just before the final program in an execline
script (that is, run it after dropping privileges and changing users and
such).

Compatibility
=============

This utility should work on most POSIX systems, though currently the setting
of the terminal size attributes is via Linux specific ioctl, and that part
will be skipped on non-Linux systems.

I've only tested it on Linux 3.13
