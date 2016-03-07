# Release checklist

## Update builtins.c

## Regenerate joe.pot

## Make sure any joerc.in changes have been made to other rc files

## Compile with maximum warnings

### Try old systems: Redhat 7.3

### Try both 32 and 64-bit systems

### Try latest gcc / clang

### Try cygwin

### Try a big-endian system (Power)

## Update NEWS.md file

## Update documentation: docs/man.md

## Update man page from man.md using ronn (maybe makefile should do this, but ronn is a pain to install)

## Update version in configure.ac, rerun ./autojoe

## Run "make dist", unpack new distribution and make sure it configures and compiles.

## Add tag to mercurial

## Install new source distribution on Sourceforge, create release announcement

## Update any links to point to the new version (maybe no longer necessary)

## Try to push latest version to downstream distributions (Cygwin, Debian, etc.)
