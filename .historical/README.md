# Historical JOE release

This snapshot contains a historical release of JOE. These versions predated
widespread use of source control and were distributed primarily on USENET
and FTP servers. As such, any related commits do not represent *real*
commits, but are included in the repository for the sake of completeness.

---

## JOE 0.1.5
**Released: 1992-02-01**

Source recovered from joe-hist archive

*Archival notes: Release date is an estimate. It is probably before Mar 26 and definitely before Jun 12.*

Author's comments:

```
Here is the last version before the big joe1.0 rewrite.  It got the be quite
sophisticated.  It now has macros and a real undo system.
```

Changelog:

```
Release 0.1.5 of JOE

FIXES

	* No longer looses single character of typehead when exiting

	* Strcat to a block not guarenteed to contain a zero fixed.  This
	  may have caused core dumps or may have messed up the help screen.

	* posix driver fixed
```

---

## JOE 0.1.4
**Released: 1992-01-30**

*The source for this release has been lost*

Changelog:

```
Release 0.1.4 of JOE

FIXES

	* Control characters in prompts now handled correctly

	* Detection for recursive macros added

	* Stdout/stderr set properly for pipe-block-through-shell-command

	* Generic SVR3 async file added

	* Spelling errors in man page fixed :-)
```

Original announcement:
```
Xref: sparky comp.editors:917 alt.religion.emacs:44 comp.unix.misc:1115 alt.os.linux:346
Newsgroups: comp.editors,alt.religion.emacs,comp.unix.misc,alt.os.linux
Path: sparky!uunet!zaphod.mps.ohio-state.edu!qt.cs.utexas.edu!news!noc.near.net!wpi.WPI.EDU!rcarter
From: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Subject: Joe Editor version 0.1.4 (you might know it as E or J)
Message-ID: <1992Jan30.191900.5741@wpi.WPI.EDU>
Sender: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Organization: Kadath Tours, Inc.
Date: Thu, 30 Jan 1992 19:19:00 GMT
Lines: 28


Well I said that the last release was going to be the last before the
rewrite, but people found enough minor bugs so that I have to make another. 
But good news!  I finally have an anonymous ftp archive site (yeah! no more
stupid uuencoding or people complaining that I posted it wrong):

	ftp 		wpi.wpi.edu   (130.215.24.1)
	login:		anonymous
	password: 	<your user name>
	cd		stusrc
	binary
	get		joe.tar.Z

The fixes for this version:

	Recursive macros detected

	Control characters typed in prompts are properly handled

	stdout/stderr are set right for the 'pipe block through shell
	command' function

	A new version of async for SVR3 (actually generic system V I think)
-- 
/*  rcarter@wpi.wpi.edu */      /* Amazing */             /* Joseph H. Allen */
int a[1817];main(z,p,q,r){for(p=80;q+p-80;p-=2*a[p])for(z=9;z--;)q=3&(r=time(0)
+r*57)/7,q=q?q-1?q-2?1-p%79?-1:0:p%79-77?1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2
]?a[p+=a[p+=q]=q]=q:0:0;for(;q++-1817;)printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);}
```

---

## JOE 0.1.3
*Release date unknown*

*The source for this release has been lost*

---

## JOE 0.1.2
**Released: 1992-01-23**

Source recovered from usenet

Original announcement:
```
Xref: funic comp.editors:3128 alt.sources:3045 alt.os.linux:131 comp.unix.xenix.misc:337
Path: funic!fuug!mcsun!uunet!caen!garbo.ucc.umass.edu!m2c!wpi.WPI.EDU!rcarter
From: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Newsgroups: comp.editors,alt.sources,alt.os.linux,comp.unix.xenix.misc
Subject: JOE Version 0.1.2
Message-ID: <1992Jan23.112219.60@wpi.WPI.EDU>
Date: 23 Jan 92 11:22:19 GMT
Sender: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Organization: Kadath Tours, Inc.
Lines: 35


Here (well on alt.sources, anyway) is what I think (hope) will be the last
version of JOE before the rewrite (version 1.0.0).  This version includes
some quick fixes and additions to the previous one: 

	* undo/redo are now a real undo system instead of undelete

	* keyboard macros added

	* repeat prefix added

	* capture shell output and pipe block through shell command added

	* A version for Linux is now included

	* Simple macros may be specified in the .joerc file

	* Some bugs in the paragraph reformat routine were fixed

	* The right margin may be specified in the .joerc file

	* A bug where strange things would happen if you typed \ in the
          search & replace prompt is fixed

I have time now so I should be able to come out with the rewrite real-soon. 
Oh, and it looks like it will have a macro language.  It's going to be C
except that the C pointers will be buffer pointers.  This won't make it more
complicated to use- the only change I envision beyond what's in 0.1.2 is
that ^K L will be able to take vi-style regular expressions and editor
commands.
-- 
/*  rcarter@wpi.wpi.edu */      /* Amazing */             /* Joseph H. Allen */
int a[1817];main(z,p,q,r){for(p=80;q+p-80;p-=2*a[p])for(z=9;z--;)q=3&(r=time(0)
+r*57)/7,q=q?q-1?q-2?1-p%79?-1:0:p%79-77?1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2
]?a[p+=a[p+=q]=q]=q:0:0;for(;q++-1817;)printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);}
```

---

## JOE 0.1.1
*Release date unknown*

*The source for this release has been lost*

Changelog:

```
FIXES AND NEW FEATURES FOR THIS VERSION

	* Left arrow jumping fixed

	* No longer touches the IXON IXOFF setting

	* Reference to Gnu EMACS removed from the copyright notice :-)

	* The patch for Undo and Redo is not needed for this version.

	* Name of initialization file changed to '.joerc'

	* Versions for ESIX and POSIX now included
	  (think you Mike Lijewski for the POSIX driver)

	* Users can now customize the help text.  The help text is now placed
	  in the initialization file

	* A compile option for passing characters with bit 7 set has been
	  added.  This is for Iceland

	* Added kill line function for the emacs people.  See 'killlin' in
          the .joerc file

	* The ioctls TIOCGSIZE and TIOCGWINSZ are used to get the
	  screen/window size.  If the window changes size, JOE resizes the
	  screen on the next key press

	* Prompts and messages wider than the screen width are now handled
	  properly
```

---

## JOE 0.1.0
**Released: 1991-09-03**

Source recovered from usenet

Original announcement:
```
Xref: funic comp.editors:2222 alt.sources:2355 alt.religion.emacs:878 comp.unix.xenix.sco:3131 comp.unix.sysv386:10434
Path: funic!fuug!mcsun!uunet!wupost!udel!sbcs.sunysb.edu!libserv1.ic.sunysb.edu!jallen
From: jallen@libserv1.ic.sunysb.edu (Joseph Allen)
Newsgroups: comp.editors,alt.sources,alt.religion.emacs,comp.unix.xenix.sco,comp.unix.sysv386
Subject: JOE Version 0.1.0
Message-ID: <1991Sep3.204511.10482@sbcs.sunysb.edu>
Date: 3 Sep 91 20:45:11 GMT
Sender: rcarter@wpi.wpi.edu
Organization: State University of New York at Stony Brook
Lines: 92
Originator: jallen@libserv1.ic.sunysb.edu
Nntp-Posting-Host: libserv1.ic.sunysb.edu

Release 0.1.0 of JOE (Joe's Own Editor):  Note that this program was
previously called 'E' (which turned out to be the RAND EDITOR and one of the
synonyms for vi) and 'J' (which turned out to be a language and the name of
another UNIX editor).  See below for list of big fixes and new features. 
Find the source for this release in 'alt.sources'

FIXES AND NEW FEATURES FOR THIS VERSION

	* Left arrow jumping fixed

	* No longer touches the IXON IXOFF setting

	* Reference to Gnu EMACS removed from the copyright notice :-)

	* The patch for Undo and Redo is not needed for this version.

	* Name of initialization file changed to '.joerc'

	* Versions for ESIX and POSIX (AIX) now included
	  (thank you Mike Lijewski for the POSIX driver)

	* Users can now customize the help text.  The help text is now placed
	  in the initialization file

	* A compile option for passing characters with bit 7 set has been
	  added.  This is for Iceland

	* Added kill line function for the emacs people.  See 'killlin' in
          the .joerc file

	* The ioctls TIOCGSIZE and TIOCGWINSZ are used to get the
	  screen/window size.  If the window changes size, JOE resizes the
	  screen on the next key press

	* Prompts and messages wider than the screen width are now handled
	  properly

Introduction

	'JOE' is a small screen editor which was designed to be easy to use
for novice users but also to be powerful and complete enough for experienced
users.  Several elements of its design are unique innovations.  Here is a copy
of the on-line help text to give you a feel for this editor: 

GO TO              DELETE    MISC      BLOCK    FIND     QUOTE    WINDOW
^B left  ^F right ^D single ^T  mode   ^KB mark ^KF text `  Ctrl  ^KO split
^Z word  ^X word  ^W >word  ^R  retype ^KK end  ^L  next ^\ bit-7 ^KI 1 / all
^A edge  ^E edge  ^O word<  ^KA center ^KC copy ^KL line FILE     ^KP up
^P up    ^N down  ^J >line  ^KJ format ^KM move EXIT     ^KD save ^KN down
^U page  ^V page  ^Y line   ^KZ shell  ^KW save ^KX save ^KR read ^KG grow
^KU top ^KV end   ^K- undo  ^K, indnt< ^KY kill ^C abort/         ^KT shrink
^G matching ([<{` ^K+ redo  ^K. indnt>             close window  ^KE get file

Other relevent features:

	* Extremely small - the XENIX version is only 58K

	* Help text can be left on while editing

	* Key layout designed to eliminate headaches: ^Q and ^S are not used,
	  both ^H and DEL are backspace.  Also, each user may customize
	  his key layout by modifying a simple initialization file

	* Versions for BSD, HPUX, POSIX (AIX), ESIX and XENIX 386 are included
	  A simple driver is also provided to ease porting to other systems

	* Currently only VT100/ANSI terminals are supported.  If the terminal
	  has scrolling regions, JOE uses them.  Has well-tuned interruptable
	  screen update algorithm

	* Has: autoindent, word-wrap, overtype/insert, picture mode (right-
	  arrow makes space past ends of lines), right margin (for paragraph
	  formatting and center), and magic tabs (the column number of text
	  after tab stops is preserved when inserting and deleting)

	* The cursor column doesn't 'jump' when you move between long and
	  short lines.  Instead the cursor column only jumps when you try to
	  edit in an invalid place or if picture mode is set, the invalid
	  place is made real by space filling

	* Editor modes can be set depending on file extension

	* No line length restrictions.  Binary files can be edited without
	  difficulty

	* Long lines are truncated, not wrapped (I.E., the screen scrolls to
	  the right to get to the truncated parts)
-- 
/*  rcarter@wpi.wpi.edu */      /* Amazing */             /* Joseph H. Allen */
int a[1817];main(z,p,q,r){for(p=80;q+p-80;p-=2*a[p])for(z=9;z--;)q=3&(r=time(0)
+r*57)/7,q=q?q-1?q-2?1-p%79?-1:0:p%79-77?1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2
]?a[p+=a[p+=q]=q]=q:0:0;for(;q++-1817;)printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);}
```

---

## J 0.0.1
**Released: 1991-08-23**

Source recovered from usenet

*Archival notes: Small patch to J*

Original announcement:
```
Xref: funic comp.editors:2115 alt.religion.emacs:840 alt.sources:2279 comp.unix.xenix.sco:3033
Path: funic!fuug!mcsun!uunet!bu.edu!m2c!wpi.WPI.EDU!rcarter
From: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Newsgroups: comp.editors,alt.religion.emacs,alt.sources,comp.unix.xenix.sco
Subject: Patch for Joe's Editor
Message-ID: <1991Aug23.012009.820@wpi.WPI.EDU>
Date: 23 Aug 91 01:20:09 GMT
Sender: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Organization: Kadath Tours, Inc.
Lines: 52


So much for my testing abilities... I work hard to test all these weird cases
and then only one day after posting the source someone finds a bug.  Oh well.

The Bug:  Running Undo or Redo before having done any deleting crashes the
editor

The fix:  Place the following lines in a file called 'patch0.0.1', change to
the directory containing the file 'j.c' and type this command:

	ed - j.c <patch0.0.1

--- Cut Here ---
/undo(/
+
a
if(!undorecs) return;
.
/redo(/
+
a
if(!undorecs) return;
.
/version/
s/0.0.0/0.0.1/
w
q
--- Cut Here ---

Then remake the editor.  The release number will be changed to 0.0.1

Also, feel free to change the name of the editor to 'JOE - Joe's Own Editor'
in case 'j' is already taken on your computer (if you have the language from
1989 or the editor from 1980).  This will be the name for the next complete
release of the editor, unless it to is taken :-)

Finally, feel free to change the name of 'keymap.j' file to '.joerc' in case
you like your initialization files hidden.  To do this, change the line

#define KEYMAP "keymap.j"

 to 

#define KEYMAP ".joerc"

This line can be found right at the beginning of the file j.h (remake the
editor after you do this).
-- 
/*  rcarter@wpi.wpi.edu */      /* Amazing */             /* Joseph H. Allen */
int a[1817];main(z,p,q,r){for(p=80;q+p-80;p-=2*a[p])for(z=9;z--;)q=3&(r=time(0)
+r*57)/7,q=q?q-1?q-2?1-p%79?-1:0:p%79-77?1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2
]?a[p+=a[p+=q]=q]=q:0:0;for(;q++-1817;)printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);}
```

---

## J 0.0.0
**Released: 1991-08-21**

Source recovered from usenet

Author's comments:

```
Here is 'J', which is not the first version of "E" released on the net, but
is pretty close.  It shows the result of some critical influence from people
on the net.  This version has:

* gap buffer

* K&R C, because it was more portable.

* multiple windows

* ~ expansion of file names

* A man page! (complete with embarrassing spelling errors :)

* Compile with 'make cruddy'.

* Still only works in 80x24 terminal

* Undelete
```

Original announcement:
```
Xref: funic alt.sources:2271 comp.editors:2096 comp.unix.xenix.sco:3018 alt.religion.emacs:834
Path: funic!fuug!mcsun!uunet!elroy.jpl.nasa.gov!usc!samsung!crackers!m2c!wpi.WPI.EDU!rcarter
From: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Newsgroups: alt.sources,comp.editors,comp.unix.xenix.sco,alt.religion.emacs
Subject: J (Joe's Editor) New Release
Message-ID: <1991Aug22.043934.1087@wpi.WPI.EDU>
Date: 22 Aug 91 04:39:34 GMT
Sender: rcarter@wpi.WPI.EDU (Randolph Carter (nee. Joseph H. Allen))
Organization: Kadath Tours, Inc.
Lines: 138



Release 0.0.0 of 'J' (Joe's Editor):  This release supersedes all previous
versions which lack a release number.  Note that this program was previously
called 'E' but the name has been changed ('E' turns out to be one of the
synonyms for 'vi').  See below for list of bug fixes and new features.
Find the 3 posts in alt.sources for the source

Introduction

	'J' is a small screen editor which was designed to be easy to use for
novice users but also to be powerful and complete enough for experienced
users.  Several elements of its design are unique innovations.  Here is a
copy of the on-line help text to give you a feel for this editor:

GO TO              DELETE    MISC      BLOCK    FIND     QUOTE    WINDOW
^B left  ^F right ^D single ^T  mode   ^KB mark ^KF text `  Ctrl  ^KO split
^Z word  ^X word  ^W >word  ^R  retype ^KK end  ^L  next ^\ bit-7 ^KI 1 / all
^A edge  ^E edge  ^O word<  ^KA center ^KC copy ^KL line FILE     ^KP up
^P up    ^N down  ^J >line  ^KJ format ^KM move EXIT     ^KD save ^KN down
^U page  ^V page  ^Y line   ^KZ shell  ^KW save ^KX save ^KR read ^KG grow
^KU top ^KV end   ^K- undo  ^K, indnt< ^KY kill ^C abort/         ^KT shrink
^G matching ([<{` ^K+ redo  ^K. indnt>             close window  ^KE get file

Other relevent features:

	* Extremely small - the XENIX version is only 58K

	* Help text can be left on while editing

	* Key layout designed to eliminate headaches: ^Q and ^S are not used,
	  both ^H and DEL are backspace.  Also, each user may customize
	  his key layout by modifying a simple initialization file

	* Versions for BSD, HPUX and XENIX 386 are included.  A simple tty
	  driver is also provided to ease porting to other systems

	* Currently only VT100/ANSI terminals are supported.  If the terminal
	  has scrolling regions, J uses them.  Has well-tuned interruptable
	  screen update algorithm

	* Has: autoindent, word-wrap, overtype/insert, picture mode (right-
	  arrow makes space past ends of lines), right margin (for paragraph
	  formatting and center), and magic tabs (the column number of text
	  after tab stops is preserved when inserting and deleting)

	* The cursor column doesn't 'jump' when you move between long and
	  short lines.  Instead the cursor column only jumps when you try to
	  edit in an invalid place or if picture mode is set, the invalid
	  place is made real by space filling

	* Editor modes can be set depending on file extension

	* No line length restrictions.  Binary files can be edited without
	  difficulty

	* Long lines are truncated, not wrapped (I.E., the screen scrolls to
	  the right to get to the truncated parts)

FIXES FOR THIS VERSION

	* Shell escape lock-ups are fixed

	* Paragraph reformat glitch is fixed

	* Cursor update screwynesses fixed for picture mode

	* Goto next word/Goto previous word/Delete word left and Delete word
	  right were made more consistant

NEW FEATURES FOR THIS VERSION

	* ~ can be used at file name prompts to expand user home directories

	* J now has undelete and redelete (you can go backwards and forewards
	  through the delete buffer to select which text you want to undelete)

	* J now has indent functions.  A marked block can be indented more
	  or less. If you use an indent function outside a marked block, the
	  lines with the same or greater indent level surrounding the cursor
	  are marked

	* Right margin was added for paragraph format and center line

	* Goto matching parenthasis (or [ { ` < )

	* ` was made the quote control character key because ^_ is not easy
	  generate on many keyboard.  `` gets `

	* tty drivers were rewritten.  This made the screen update faster
	  (now the screen update is instantaneous on 20MHz 386s).  Screen
	  update preempting also works better

THE PLAN

	Version 1.0.0 will be a complete rewrite.  It should be done sometime
before january.  These are my goals for it:

	* Block oriented software virtual memory support (for less thrashing
	  and elimination of file size limits caused by the system's process
	  size limit)

	* MS-DOS support

	* X windows support

	* Better integration with UNIX:  J will be able to process text
	  through pipes (like vi), will have standard unix regular expressions,
	  and will be usable as a 'more' program.

	* Will be terminal independant and use termcap and terminfo

	* Will have a ASCII/HEX dump display mode

	* Will have complete program developement support (save&make, next
	  error/previous error, tags file support)

	* Will have a much more powerful scrolling algorithm.  I have a
	  methode which is simpler and faster than even GNU-EMACS' Gosling
	  algorithm

	* All of this will require no changes to the basic keyboard layout-
	  maybe one more added line of help text for new things

	* It will probably have keyboard macros but it won't have much of
	  an extension language.  Maybe in version 2.0.0

	* I expect all of this not to add more than 30 or 40% to the size

		Joseph H. Allen
		28 Dale Lane
		Smithtown, N.Y. 11787

-- 
/*  rcarter@wpi.wpi.edu */      /* Amazing */             /* Joseph H. Allen */
int a[1817];main(z,p,q,r){for(p=80;q+p-80;p-=2*a[p])for(z=9;z--;)q=3&(r=time(0)
+r*57)/7,q=q?q-1?q-2?1-p%79?-1:0:p%79-77?1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2
]?a[p+=a[p+=q]=q]=q:0:0;for(;q++-1817;)printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);}
```

---

## E 0.0
**Released: 1988-09-20**

Source recovered from joe-hist archive

Author's comments:

```
This is close to the very first version of JOE (or E as it was first
called).  It has:

 * Only a single window.

 * No ~ expansion of file names.

 * Block selection is not quite like WordStar.  You set the mark with ^KB
   and then hit ^KM to cut the text between the mark and the cursor.  In
   this case, the cut text is saved in a file called "cut.c".  You hit ^T to
   insert the cut.e file at the cursor.

 * The terminal screen must be 80 x 24.

 * VT52 terminals are handled, but you have to recompile.

 * It does have word wrap, paragraph reformat, autoindent and
   picture mode.

 * No status line.

 * It does have a single page of on-line help: hit ^K H to toggle it.

 * The entire editor is a single 2800 line C file: e.c

 * Does not have gap buffer yet.

 * Note that E is ANSI-C!  I later reverted it to K&R C for more
   enhanced portability.
```