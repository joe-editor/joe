/* Syntax highlighting functions */

#include "config.h"
#include "types.h"
#include "b.h"
#include "scrn.h"

/* Given starting state and a line, return ending state and colors to use for the line */

#define stIDLE 0		/* Initial state */
#define stCOMMENT 1		/* In a comment */
#define stCOMMENTe 3		/* In a comment plus * (tmp) */
#define stIDLEsl 4		/* IDLE plus / (tmp) */
#define stLCOMMENT 5		/* In a // comment (tmp) */
#define stCCONST 6
#define stSCONST 7
#define stAND 8
#define stOR 9
#define stEQ 10
#define stNOT 11
#define stGT 12
#define stLT 13
#define stMOD 14
#define stXOR 15
#define stMUL 16
#define stSUB 17
#define stADD 18
#define stSHR 19

int parse_c(int state,int *out,P *p)
{
int c;
int x=0;

for(x=0;x!=1024;++x)
  out[x]=FG_WHITE;
x=0;

for(c=pgetc(p);;c=pgetc(p))
  switch(state)
    {
    case stAND:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '&':
        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stOR:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '|':
        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stEQ:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stNOT:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stGT:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        case '>':
          {
          state = stSHR;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stLT:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        case '<':
          {
          state = stSHR;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stMOD:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stXOR:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stMUL:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stSUB:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stADD:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stSHR:
      {
      switch(c)
        {
        case '\n':
        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '=':
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }

        default:
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          goto ahead;
          }
        }
      break;
      }

    case stIDLE:
      {
      ahead:
      switch(c)
        {
        case '\n':
          return state;

        case NO_MORE_DATA:
          return state;

        case ' ': case '\t': case '\r':
          {
          out[x++]=FG_WHITE;
          break;
          }

        case '/':
          {
          state = stIDLEsl;
          break;
          }

        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
          {
          out[x++]=FG_WHITE;
          break;
          }

        case '~':
          {
          out[x++]=FG_WHITE;
          break;
          }

        case '&': /* could be && or &= */
          {
          state = stAND;
          break;
          }

        case '|': /* could be || or |= */
          {
          state = stOR;
          break;
          }

        case '=': /* could be = or == */
          {
          state = stEQ;
          break;
          }

        case '!': /* could be ! or != */
          {
          state = stNOT;
          break;
          }

        case '>': /* could be >> or >= */
          {
          state = stGT;
          break;
          }

        case '<': /* could be < or <= */
          {
          state = stLT;
          break;
          }

        case '%': /* could be % or %= */
          {
          state = stMOD;
          break;
          }

        case '^': /* could be ^ or ^= */
          {
          state = stXOR;
          break;
          }

        case '*': /* could be * or *= */
          {
          state = stMUL;
          break;
          }

        case '-': /* could be - or -= */
          {
          state = stSUB;
          break;
          }

        case '+': /* could be + or += */
          {
          state = stADD;
          break;
          }

        case '\'': /* character constant */
          {
          out[x++]=FG_MAGENTA;
          state = stCCONST;
          break;
          }

        case '"': /* string constant */
          {
          out[x++]=FG_MAGENTA;
          state = stSCONST;
          break;
          }

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          {
          int y;

          nConst:
          y=x;

          lp:
          do
            {
            ++x;
            c=pgetc(p);
            }
            while(c>='0' && c<='9');

          if(c=='x' || c=='X')
            { /* Hex constant */
            goto lp;
            }
          else if(c=='.')
            { /* Second half of floating follows */
            goto lp;
            }
          else if(c=='U')
            { /* U for unsigned */
            ++x;
            c=pgetc(p);
            goto done;
            }
          else if(c=='e' || c=='E')
            { /* Third half of floating follows */
            ++x;
            c=pgetc(p);
            if(c=='+' || c=='-')
              goto lp;
            else if(c>='0' && c<='9')
              goto lp;
            }

          done:
          while(y!=x) out[y++]=FG_MAGENTA;
          goto ahead;
          break;
          }

        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
        case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case '_':
          {
          int y=x;
          char buf[20];
          int n;
          buf[0]=c;
          ++x;
          /* Reads ahead */
          while((c=pgetc(p)), (c>='a' && c<='z' || c>='A' && c<='Z' || c=='_' || c>='0' && c<='9'))
            if(x-y<18)
              buf[x++-y]=c;
            else
              x++;
          buf[x-y]=0;
          if(!strcmp(buf,"int") ||
             !strcmp(buf,"float") ||
             !strcmp(buf,"long") ||
             !strcmp(buf,"short") ||
             !strcmp(buf,"char") ||
             !strcmp(buf,"double") ||
             !strcmp(buf,"signed") ||
             !strcmp(buf,"unsigned") ||
             !strcmp(buf,"void") ||
             !strcmp(buf,"static") ||
             !strcmp(buf,"extern") ||
             !strcmp(buf,"register") ||
             !strcmp(buf,"volatile") ||
             !strcmp(buf,"inline") ||
             !strcmp(buf,"automatic"))
            n=FG_GREEN;
          else if(!strcmp(buf,"if") ||
             !strcmp(buf,"else") ||
             !strcmp(buf,"while") ||
             !strcmp(buf,"for") ||
             !strcmp(buf,"break") ||
             !strcmp(buf,"continue") ||
             !strcmp(buf,"do") ||
             !strcmp(buf,"case") ||
             !strcmp(buf,"default") ||
             !strcmp(buf,"switch") ||
             !strcmp(buf,"goto"))
            n=FG_YELLOW;
          else
            n=FG_WHITE;
          while(y!=x) out[y++]=n;
          goto ahead;
          break;
          }

        case '#': /* Could't be preprocessor directive if it's first thing on line */
          { /* Come back to this */
          out[x++]=FG_WHITE;
          break;
          }

        case ';': /* at end of statements */
          {
          out[x++]=FG_WHITE;
          break;
          }

        case ':': /* for labels and case and ? */
          {
          out[x++]=FG_WHITE;
          break;
          }

        case ',':
          {
          out[x++]=FG_WHITE;
          break;
          }

        case '.': /* could be constant or separator */
          {
          c=pgetc(p);
          if(c>='0' && c<='9')
            {
            goto nConst;
            }
          else
            {
            out[x++]=FG_WHITE;
            goto ahead;
            }
          }

        case '?': /* ? operator */
          {
          out[x++]=FG_WHITE;
          break;
          }

        default: /* ` and @ are unused in C.  \ at end of line... */
          {
          out[x++]=FG_WHITE;
          break;
          }
        }
      break;
      }

    case stCCONST:
      {
      switch(c)
        {
        case NO_MORE_DATA:
          {
          out[x++]=FG_MAGENTA;
          return stIDLE;
          }

        case '\n':
          {
          out[x++]=FG_MAGENTA;
          return stIDLE;
          }

        case '\'':
          {
          out[x++]=FG_MAGENTA;
          state = stIDLE;
          break;
          }

        case '%':
          {
          out[x++]=FG_RED;
          lp1:
          c=pgetc(p);
          switch(c)
            {
            case NO_MORE_DATA:
            case '\n':
              return stIDLE;

            case '\'':
              {
              out[x++]=FG_MAGENTA;
              state = stIDLE;
              break;
              }

            case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
            case 'a': case 'A': case 'c': case 's': case 'p': case 'n':
            case '%': case 'S': case 'C':
              {
              out[x++]=FG_RED;
              break;
              }

            default:
              {
              out[x++]=FG_RED;
              goto lp1;
              }
            }
          break;
          }

        case '\\':
          {
          out[x++]=FG_RED;
          c=pgetc(p);
          if(c==NO_MORE_DATA)
            {
            out[x++]=FG_MAGENTA;
            return stIDLE;
            }
          else if(c=='\n')
            {
            out[x++]=FG_MAGENTA;
            return stCCONST;
            }
          else
            {
            out[x++]=FG_RED;
            }
          break;
          }

        default:
          {
          out[x++]=FG_MAGENTA;
          break;
          }
        }
      break;
      }

    case stSCONST:
      {
      switch(c)
        {
        case NO_MORE_DATA:
          {
          out[x++]=FG_MAGENTA;
          return stIDLE;
          }

        case '\n':
          {
          out[x++]=FG_MAGENTA;
          return stIDLE;
          }

        case '\"':
          {
          out[x++]=FG_MAGENTA;
          state = stIDLE;
          break;
          }

        case '%':
          {
          out[x++]=FG_RED;
          lp2:
          c=pgetc(p);
          switch(c)
            {
            case NO_MORE_DATA:
            case '\n':
              return stIDLE;

            case '\'':
              {
              out[x++]=FG_MAGENTA;
              state = stIDLE;
              break;
              }

            case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
            case 'a': case 'A': case 'c': case 's': case 'p': case 'n':
            case '%': case 'S': case 'C':
              {
              out[x++]=FG_RED;
              break;
              }

            default:
              {
              out[x++]=FG_RED;
              goto lp2;
              }
            }
          break;
          }

        case '\\':
          {
          out[x++]=FG_MAGENTA;
          c=pgetc(p);
          if(c==NO_MORE_DATA)
            {
            out[x++]=FG_MAGENTA;
            return stIDLE;
            }
          else if(c=='\n')
            {
            out[x++]=FG_MAGENTA;
            return stSCONST;
            }
          else
            {
            out[x++]=FG_MAGENTA;
            }
          break;
          }

        default:
          {
          out[x++]=FG_MAGENTA;
          break;
          }
        }
      break;
      }

    case stIDLEsl:
      {
      switch(c)
        {
        case '*':
          {
          state = stCOMMENT;
          out[x++]=FG_CYAN;
          out[x++]=FG_CYAN;
          break;
          }

        case '/':
          {
          state = stLCOMMENT;
          out[x++]=FG_CYAN;
          out[x++]=FG_CYAN;
          break;
          }

        case NO_MORE_DATA:
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        case '\n':
          {
          out[x++]=FG_WHITE;
          return stIDLE;
          }

        default:
          {
          out[x++]=FG_WHITE;
          out[x++]=FG_WHITE;
          state = stIDLE;
          break;
          }
        }
      break;
      }

    case stLCOMMENT:
      {
      switch(c)
        {
        case '\n':
          return stIDLE;

        case NO_MORE_DATA:
          return stIDLE;

        default:
          {
          out[x++]=FG_CYAN;
          break;
          }
        }
      break;
      }

    case stCOMMENT:
      {
      switch(c)
        {
        case '\n':
          return state;

        case NO_MORE_DATA:
          return state;

        case '*':
          {
          out[x++]=FG_CYAN;
          state = stCOMMENTe;
          break;
          }

        default:
          {
          out[x++]=FG_CYAN;
          break;
          }
        }
      break;
      }

    case stCOMMENTe:
      {
      switch(c)
        {
        case '\n':
          return stCOMMENT;

        case NO_MORE_DATA:
          return stCOMMENT;

        case '/':
          {
          out[x++]=FG_CYAN;
          state = stIDLE;
          break;
          }

        case '*':
          {
          out[x++]=FG_CYAN;
          break;
          }

        default:
          {
          out[x++]=FG_CYAN;
          state = stCOMMENT;
          break;
          }
        }
      break;
      }
    }
}
