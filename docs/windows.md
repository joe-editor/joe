# JOE for Windows tips

### Clipboard integration

^K C in JOE and JSTAR will paste text from the Windows clipboard if no block
is set.  JOE also maps ^S to copy the current block into the clipboard.  If
you use a different personality, you may use the context menu (right click
in the editor) to copy into/paste from the Windows clipboard.

### Explorer integration

You can drag files from Windows explorer onto JOE, and it will open those
files.

### Appearance

JOE for Windows comes with several builtin color schemes, which are more
suitable for a GUI environment.  They can be selected through the context
menu (right click on the editor window).  The font and cursor type may also
be changed there.

### Full screen

You can toggle fullscreen mode with Alt-Enter to relive the old days.  Even
better, find a CP437 font!

### Pop-up terminals

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
the "-i" switch.  The shell windows interact with applications similarly to
the way that mintty does.  If there is a workaround for mintty, it will
likely work for JOE.

### File locations

JOE for Windows uses a different (but similar) file layout than JOE for
UNIXen.  Most files important to the editor are found at the install
location of the joe.exe file, usually C:\\Program Files (x86)\\JoeEditor. 
\*rc files are stored in the conf\\ subdirectory, \*.jsf files in syntax\\ and
color schemes in schemes\\.

You can override any of these files or settings by sticking a copy in your
"home" directory.  This directory is found in %LOCALAPPDATA%\\JoeEditor
(where %LOCALAPPDATA% is the value of that environment variable -- it's
usually C:\\Users\\(your user name)\\AppData\\Local\\JoeEditor).

* Place any \*rc files in the root of that directory

* Place any \*.jsf files in the syntax\\ subdirectory

* Place any \*.joecolor (color scheme) files in the schemes\\ subdirectory

### What is not implemented or different?

JOE for Windows strives for feature parity with mainline JOE, but some
things have not been implemented:

* File locks
* File open/save pipe syntax (load/save "!command")
* Environment variables are completely ignored.  Most of them don't map well
to Windows anyhow.
* Probably more?
