/* Messages
   Copyright (C) 1992 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software 
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
details.  

You should have received a copy of the GNU General Public License along with 
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 
675 Mass Ave, Cambridge, MA 02139, USA.  */ 

#include "config.h"
#include "msgs.h"

char M001[]="DEADJOE";
char M002[]="\n*** Modified files in JOE when it aborted on %s";
char M003[]="*** JOE was aborted by signal %d\n";
char M004[]="*** JOE was aborted because the terminal closed\n";
char M005[]="\n*** File '%s'\n";
char M006[]="\n*** File '(Unnamed)'\n";
char M007[]="** ROW=%ld COL=%ld BYTE=%ld/0x%lX **";
char M008[]="** ROW=%ld COL=%ld BYTE=%ld/0x%lX CHAR=%d/0%o/0x%X **";
char M009[]="Error writing file";
char M010[]="Error opening file";
char M011[]="Error seeking file";
char M012[]="Error reading file";
char M013[]="New File";
char M014[]="No block";
char M015[]="Name of file to write (^C to abort): ";
char M016[]="Command to filter block through (^C to abort): ";
char M017[]="Couldn't make backup file... file not saved";
char M018[]="Name of file to save (^C to abort): ";
char M019[]="Lose changes to this file (y,n,^C)? ";
char M020[]="Name of file to edit (^C to abort): ";
char M021[]="Name of file to insert (^C to abort): ";
char M022[]="File ";
char M023[]=" saved.";
char M024[]=" not changed so no update needed.";
char M025[]="Invalid line number";
char M026[]="Go to line (^C to abort): ";
char M027[]="Ctrl-";
char M028[]="Meta-";
char M029[]="Meta-Ctrl-";
char M030[]="Couldn't open tags file";
char M031[]="Not found";
char M032[]="Tag string to find (^C to abort): ";
char M033[]="Rectangle mode";
char M034[]="Text-stream mode";
char M035[]="Autoindent enabled";
char M036[]="Autoindent disabled";
char M037[]="Word wrap enabled";
char M038[]="Word wrap disabled";
char M039[]="Insert mode";
char M040[]="Overtype mode";
char M041[]="Cursor will be recentered when window scrolls";
char M042[]="Cursor will not be recentered when window scrolls";
char M043[]="Last line forced to have NL when file saved";
char M044[]="Last line not forced to have NL";
char M045[]="Characters above 127 displayed as-is";
char M046[]="Characters above 127 remapped";
char M047[]="Left margin %ld (^C to abort): ";
char M048[]="Indent char %d (SPACE=32, TAB=9, ^C to abort): ";
char M049[]="Indent step %ld (^C to abort): ";
char M050[]="Right margin %ld (^C to abort): ";
char M051[]="Tab width %d (^C to abort): ";
char M052[]="Lines to keep for PgUp/PgDn or -1 for 1/2 (%ld): ";
char M053[]="No. times to repeat next command (^C to abort): ";
char M054[]="** Program finished **\n";
char M055[]="Program already running in this window";
char M056[]="No ptys available";
char M057[]="\rOut of memory\r\n";
char M058[]="Processing keymap file '%s'...";
char M059[]="\n%s %d: Unknown option";
char M060[]="\n%s %d: No pattern selected for option";
char M061[]="\n%s %d: End of joerc file occured before end of help text";
char M062[]="\n%s %d: Unknown context";
char M063[]="\n%s %d: Key function '%s' not found";
char M064[]="\n%s %d: No context selected for key";
char M065[]="\ndone\n";
char M066[]="done\n";
char M067[]="Macro to record (0-9 or ^C to abort): ";
char M068[]="Couldn't open file '%s'\n";
char M069[]="\\i** Joe's Own Editor v1.0.12 ** Copyright (C) 1994 Joseph H. Allen **\\i";
char M070[]="Replace with (^C to abort): ";
char M071[]="(I)gnore case (R)eplace (B)ackwards NNN (^C to abort): ";
char M072[]="Find (^C to abort): ";
char M073[]="Replace (Y)es (N)o (R)est (^C to abort)?";
char M074[]="Couldn't load termcap/terminfo entry\n";
char M075[]="Sorry, your terminal can't do absolute cursor positioning\nIt's broken\n";
char M076[]="Couldn't read directory ";
char M077[]="%s is out of date\n";
char M078[]="Couldn't open tty\n";
char M079[]="You are at the command shell.  Type 'exit' to return\n";
char M080[]="You have suspended the program.  Type 'fg' to return\n";
char M081[]="(Unnamed)";
char M082[]=" (Modified)";
char M083[]=" (Macro %d recording...)";
char M084[]="*SHELL* Ctrl-K H for help";
char M085[]="Ctrl-K H for help";
char M086[]="Kill program (y,n,^C)?";
char M087[]=" not saved.";
char M088[]="File (Unnamed) not saved.";
char M089[]=" not changed so no update needed.";
char M090[]="File (Unnamed) not changed so no update needed.";
