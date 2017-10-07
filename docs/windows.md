# JOE for Windows tips

JOE for Windows is a native port of Joe's Own Editor.  Unlike the Cygwin
version, it is written (more-or-less) directly against the Windows API and
does not need a large, external compatibility layer.  It is compiled with
the standard Microsoft compiler and has no external dependencies.  Changes
were made to better integrate JOE into a Windows environment, such as a
more Windows-friendly file layout and mouse support.  Other additions include
drag-and-drop and clipboard integration.

## Clipboard integration

`^K C` in JOE and JSTAR will paste text from the Windows clipboard if no block
is set.  JOE also maps `^S` to copy the current block into the clipboard.  If
you use a different personality, you may use the context menu (right click
in the editor) to copy into/paste from the Windows clipboard.

## Explorer integration

You can drag files from Windows explorer onto JOE, and it will open those
files.

## Appearance

JOE for Windows comes with several builtin color schemes, which are more
suitable for a GUI environment.  They can be selected through the context
menu (right click on the editor window).  The font and cursor type may also
be changed there.

## Pop-up terminals

JOE 4.0 introduced pop-up virtual terminals accessible via the `F1`-`F4` keys.
These terminals are improved over the previous type (which were invoked with
`^K '`) in that they interpret ANSI codes and can update any part of the
screen.  This is (partially) supported in the Windows version with the help
of [winpty](https://github.com/rprichard/winpty) (by Ryan Prichard).

By default, the standard Windows command interpreter (`cmd`) is loaded when you
start a new virtual terminal, but this is configurable by modifying the
`vt.bat` script.  It can be made to run Powershell or Cygwin out-of-the-box, or
another shell with a little modification.  Powershell and `cmd` both require
winpty (which screen-scrapes a hidden console), whereas Cygwin can be used
more <i>directly</i> through a pseudo terminal with the help of the `socat`
package. (`socat` does not come installed by default on Cygwin, so if you plan
to use it you should install it.)

Generally, you should copy the `vt` directory to your local user directory and
modify `vt.bat` to choose your shell (instructions can be found at the top of
that file).

    xcopy /S /I "C:\Program Files (x86)\JoeEditor\vt" "%LOCALAPPDATA%\JoeEditor\vt"
    joe "%LOCALAPPDATA%"\JoeEditor\vt\vt.bat

There is currently a bug in winpty that does not propagate `^C` to some programs
(most notably, the shell).  Attempts to fix this delayed a couple of releases,
so as of now it is currently broken.  This, however, does not apply to Cygwin.

### Old-style shell windows

Shell windows made with `^K '` work, but with some limitations.  Programs that
interact directly with the console interface (e.g., PowerShell) do not work
and will appear to hang when run.  You may be able to stop them by moving the
cursor away from the end of file and pressing ^C to terminate.  If that doesn't
work, try task manager.

Some programs can be made to work, for example Python will behave if given
the `-i` switch.  The shell windows interact with applications similarly to
the way that mintty does.  If there is a workaround for mintty, it will
likely work for JOE.

## Standalone version

JOE for Windows comes in two installation flavors: Windows installer (MSI) and
standalone EXE.  You can find standalone versions on [sourceforge](https://sourceforge.net/projects/joe-editor/files/JOE%20for%20Windows/),
in the `standalone` directories for releases since 4.1.  There is one per flavor,
each with every syntax and rc file baked into it, so that no external files are
required for operation.

Currently, the only limitation present in the standalone version is that pop-up
virtual terminals, mentioned in the above section, are not supported.  They can
be made to work by placing a `vt` directory where JOE would normally expect to
find one, but you'd need to get it from the normal install.

### Customizing standalone binaries

Since the `joerc` file is embedded within the EXE, it is a little less
straightforward to customize the standalone binaries.  They will still try
to read `conf\joerc` (or `jmacsrc`, `jpicorc`, etc) relative to the EXE file
as well as in your home directory as normal (see below), so it can be
overridden by placing an rc file in one of those locations.

However, that defeats the purpose of having a standalone version, and it is
possible to easily customize the EXE to your liking.  JOE for Windows embeds
its required files as *resources* in the EXE.  It is possible to change a
file's resources without recompiling it.  There is a tool to do this that is
known to work: [Resource Hacker](http://www.angusj.com/resourcehacker/).

Once Resource Hacker is installed, here are the general steps to customize
your executable:

1. Make a backup of the standalone EXE file.
2. Right click it in Windows Explorer, and select "Open using Resource Hacker"
3. In the left pane, expand the `RCData` section
4. Find all of the embedded files in this section, prefixed with `F:*`
5. Expand the file you want to edit/replace and select the lone entry beneath
it, `1033`
6. You will see the current contents in the right pane.  Note that many files
are compressed so you will see a hex dump for those (but they can safely be
replaced without you having to compress your version)
7. Right click on `1033` and select `Replace Resource`.  This will bring up a
file picker and you can select the file you'd like to insert.
8. When you are finished, select `Save As` from the file menu to generate your
customized `joe.exe`.

## File locations

JOE for Windows uses a different (but similar) file layout than JOE for
UNIXen.  Most files important to the editor are found at the install
location of the joe.exe file, usually `C:\Program Files (x86)\JoeEditor`. 
\*rc files are stored in the `conf\` subdirectory, \*.jsf files in `syntax\` and
color schemes in `colors\`.

You can override any of these files or settings by sticking a copy in your
"home" directory.  This directory is found in `%LOCALAPPDATA%\JoeEditor`
(where `%LOCALAPPDATA%` is the value of that environment variable -- it's
usually `C:\Users\You\AppData\Local\JoeEditor`).

* Place any \*rc files in the root of that directory

* Place any \*.jsf files in the `syntax\` subdirectory

* Place any \*.jcf (color scheme) files in the `colors\` subdirectory

## Full screen

You can toggle fullscreen mode with Alt-Enter to relive the old days.  Even
better, find a CP437 font!

## What is not implemented or different?

JOE for Windows strives for feature parity with mainline JOE, but some
things have not been implemented:

* File locks
* File open/save pipe syntax (load/save "!command")
* Environment variables are completely ignored.  Most of them don't map well
to Windows anyhow.
* Probably more?
