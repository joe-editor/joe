
                              Key map for e editor

 E looks for this file in:
        1 - Current directory
        2 - User's home directory
        3 - /usr/bin
 
 ******************************************************************************
 * Notes about keymap syntax:                                                 *
 *                                                                            *
 * Any line beginning with whitespace is a comment line.  At least 2          *
 * characters of whitespace past the end of definition line also begin a      *
 * comment.                                                                   *
 *                                                                            *
 * There are two types of lines as follows:                                   *
 *                                                                            *
 * :<context>                   This gives the name of a context.  Bindings   *
 *                              following this line are added into this       *
 *                              context.                                      *
 *                                                                            *
 * <function> <key> <key> ...   This binds a key sequence to a function       *
 *                                                                            *
 * <key> is any single character or                                           *
 * sp    for space or                                                         *
 * ^     followed by characters in the range @ - _ for control characters or  *
 * ^?    for del                                                              *
 *                                                                            *
 * Any number of whitespace characters may appear between <function> and the  *
 * first <key>, but there must be single spaces between <key>s.               *
 *                                                                            *
 ******************************************************************************

 File name dependant mode settings

   a   autoindent (indent)
   o   overwrite  (insert)
   p   picture mode (no picture mode)
   w   word wrap (no word wrap)
   t   tab magic (no tab magic)

 Note that when there ambiguities (extensions which begin the same), the last
 one is chosen.

*       wt              Default for unknown extensions.  This must be first
*.c     at              C programs
*.h     at              C header files
*.p     at              Pascal programs
*.f     at              Fortran programs
*.e     at              This file

 For the E editor there is currently only one context:

:main

backs           ^H              Backspace
backs		^?
backword        ^O              Backspace word
bof             ^K U            Move cursor to beginning of file
bof             ^K ^U
bof             ^K u
bol             ^A              Move cursor to beginning of line
bol             ^[ [ H
bol		^K ^[ [ H
bol		^[ O P
bol		^K ^[ O P
cpyblk          ^K C            Copy previously marked block
cpyblk          ^K ^C
cpyblk          ^K c
ctrlin          ^K A            Center line
ctrlin          ^K ^A
ctrlin          ^K a
delblk          ^K Y            Delete previously marked block
delblk          ^K ^Y
delblk          ^K y
delch           ^D		Delete character
deleol          ^J              Delete to end of line
dellin          ^Y              Delete entire line
dnarw           ^N              Move cursor down
dnarw           ^[ [ B
dnarw		^[ O B
dnarw		^K ^[ [ B
dnarw		^K ^[ O B
eexit           ^C              Exit (window/file)
eexit           ^K Q
eexit           ^K q
eof             ^K V            Move cursor to end of file
eof             ^K ^V
eof             ^K v
eol             ^E              Move cursor to end of line
eol             ^[ [ F
eol		^K ^[ [ F
eol		^[ O S
eol		^K ^[ O S
exsave          ^K X            Save & exit
exsave          ^K ^X
exsave          ^K x
findfirst       ^K F            Find text
findfirst       ^K ^F
findfirst       ^K f
findnext        ^L              Find next
findline        ^K L            Goto line number
findline        ^K ^L
findline        ^K l
gotomatching	^G
indentl		^K ,		Indent left
indentr		^K .		Indent right
insfil          ^K R            Insert a file
insfil          ^K ^R
insfil          ^K r
 inss            ^G              Insert a space
inss            ^[ [ L
inss		^K ^[ [ L
killword        ^W              Delete word under cursor
ltarw           ^B              Move cursor left
ltarw           ^[ [ D
ltarw		^K ^[ [ D
ltarw		^[ O D
ltarw		^K ^[ O D
mode            ^T              Change mode
moveblk         ^K M            Move previously marked block
moveblk         ^K ^M
moveblk         ^K m
pgdn            ^V              Move cursor 1/2 page down
pgdn            ^[ [ G
pgdn		^K ^[ [ G
pgdn            ^[ [ 6 ~
pgdn		^K ^[ [ 6 ~
pgup            ^U              Move curosr 1/2 page up
pgup            ^[ [ I
pgup		^K ^[ [ I
pgup		^[ [ 5 ~
pgup		^K ^[ [ 5 ~
push            ^K Z            Push to a shell
push            ^K ^Z
push            ^K z
redo		^K +		Redo
reformat        ^K J            Reformat paragraph
reformat        ^K ^J
reformat        ^K j
rewrite         ^R              Retype screen
rtarw           ^F              Move cursor right
rtarw           ^[ [ C
rtarw		^K ^[ [ C
rtarw		^[ O C
rtarw		^K ^[ O C
rtn             ^M              The return key
saveit          ^K D            Save file
saveit          ^K ^D
saveit          ^K d
saveit          ^K S
saveit          ^K s
setbeg          ^K B            Set beginning of block
setbeg          ^K ^B
setbeg          ^K b
setend          ^K K            Set end of block
setend          ^K ^K
setend          ^K k
stquote         `               Next character is a control character
stquote8th      ^\              Next character has 7th bit set
thelp           ^K H            Toggle help screen on/off
thelp           ^K ^H
thelp           ^K h
thelp           ^[ [ 2 8 ~
thelp		^K ^[ [ 2 8 ~
undo		^K -		Undelete
uparw           ^P              Move cursor up
uparw           ^[ [ A
uparw		^K ^[ [ A
uparw		^[ O A
uparw		^K ^[ O A
wedit           ^K E            Edit another file
wedit           ^K ^E
wedit           ^K e
wexplode        ^K I            Zoom window
wexplode        ^K ^I
wexplode        ^K i
wgrow           ^K G            Grow window
wgrow           ^K ^G
wgrow           ^K g
wnext           ^K N            Goto next window
wnext           ^K ^N
wnext           ^K n
wprev           ^K P            Goto previous window
wprev           ^K ^P
wprev           ^K p
wrdl            ^Z              Move to previous word
wrdl		^[ O Q
wrdl		^K ^[ O Q
wrdr            ^X              Move to next word
wrdr		^[ O R
wrdr		^K ^[ O R
writeblk        ^K W            Write previously marked block to a file
writeblk        ^K ^W
writeblk        ^K w
wshrink         ^K T            Shrink window
wshrink         ^K ^T
wshrink         ^K t
wsplit          ^K O            Split window
wsplit          ^K ^O
wsplit          ^K o

type            ^I              Tab

type            sp              Typable characters
type            !
type            "
type            #       
type            $
type            %
type            &
type            '
type            (
type            )
type            *
type            +
type            ,
type            -
type            .
type            /
type            0
type            1
type            2
type            3
type            4
type            5
type            6
type            7
type            8
type            9
type            :
type            ;
type            <
type            =
type            >
type            ?
type            @
type            A
type            B
type            C
type            D
type            E
type            F
type            G
type            H
type            I
type            J
type            K
type            L
type            M
type            N
type            O
type            P
type            Q
type            R
type            S
type            T
type            U
type            V
type            W
type            X
type            Y
type            Z
type            [
type            \
type            ]
type            ^
type            _
type            a
type            b
type            c
type            d
type            e
type            f
type            g
type            h
type            i
type            j
type            k
type            l
type            m
type            n
type            o
type            p
type            q
type            r
type            s
type            t
type            u
type            v
type            w
type            x
type            y
type            z
type            {
type            |
type            }
type            ~
