# Historical JOE release

This snapshot contains a historical release of JOE. These versions predated
widespread use of source control and were distributed primarily on USENET
and FTP servers. As such, any related commits do not represent *real*
commits, but are included in the repository for the sake of completeness.

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