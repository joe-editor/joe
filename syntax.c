/*
 *	Syntax highlighting DFA interpreter
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "scrn.h"
#include "utils.h"
#include "hash.h"
#include "charmap.h"
#include "syntax.h"

/* Parse one line.  Returns new state.
   'syntax' is the loaded syntax definition for this buffer.
   'line' is advanced to start of next line.
   Global array 'attr_buf' end up with coloring for each character of line.
   'state' is initial parser state for the line (0 is initial state).
*/

int *attr_buf = 0;
int attr_size = 0;

HIGHLIGHT_STATE parse(struct high_syntax *syntax,P *line,HIGHLIGHT_STATE h_state)
{
	struct high_state *h = syntax->states[h_state.state];
			/* Current state */
	unsigned char buf[24];	/* Name buffer (trunc after 23 characters) */
	int buf_idx=0;	/* Index into buffer */
	int c;		/* Current character */
	int *attr_end = attr_buf+attr_size;
	int *attr = attr_buf;
	int buf_en = 0;	/* Set for name buffering */
	int ofst = 0;	/* record offset after we've stopped buffering */

	buf[0]=0;	/* Forgot this originally... took 5 months to fix! */

	/* Get next character */
	while((c=pgetc(line))!=NO_MORE_DATA) {
		struct high_cmd *cmd, *kw_cmd;
		int x;

		/* Hack so we can have UTF-8 characters without crashing */
		if (c < 0 || c > 255)
			c = 0x1F;

		/* Expand attribute array if necessary */
		if(attr==attr_end) {
			attr_buf = joe_realloc(attr_buf,sizeof(int)*(attr_size*2));
			attr = attr_buf + attr_size;
			attr_size *= 2;
			attr_end = attr_buf + attr_size;
		}

		/* Advance to next attribute position (note attr[-1] below) */
		attr++;

		/* Loop while noeat */
		do {
			/* Color with current state */
			attr[-1] = h->color;
			/* Get command for this character */
			if (h->delim && c == h_state.saved_s[0])
				cmd = h->delim;
			else
				cmd = h->cmd[c];
			/* Determine new state */
			if (cmd->delim && !strcmp(h_state.saved_s,buf)) {
				cmd = cmd->delim;
				h = cmd->new_state;
				/* Recolor string delimiter */
				for(x= -(buf_idx+1);x<-1;++x)
					attr[x-ofst] = h -> color;
			} else if (cmd->keywords && (cmd->ignore ? (kw_cmd=htfind(cmd->keywords,lowerize(buf))) : (kw_cmd=htfind(cmd->keywords,buf)))) {
				cmd = kw_cmd;
				h = cmd->new_state;
				/* Recolor keyword */
				for(x= -(buf_idx+1);x<-1;++x)
					attr[x-ofst] = h -> color;
			} else {
				h = cmd->new_state;
			}
			/* Recolor if necessary */
			for(x=cmd->recolor;x<0;++x)
				attr[x] = h -> color;

			/* Save string? */
			if (cmd->save_s)
				strcpy((char *)h_state.saved_s,(char *)buf);

			/* Save character? */
			if (cmd->save_c) {
				if (c=='<')
					h_state.saved_s[0] = '>';
				else if (c=='(')
					h_state.saved_s[0] = ')';
				else if (c=='[')
					h_state.saved_s[0] = ']';
				else if (c=='{')
					h_state.saved_s[0] = '}';
				else if (c=='`')
					h_state.saved_s[0] = '\'';
				else
					h_state.saved_s[0] = c;
			}

			/* Start buffering? */
			if (cmd->start_buffering) {
				buf_idx = 0;
				buf_en = 1;
				ofst = 0;
			}

			/* Stop buffering? */
			if (cmd->stop_buffering)
				buf_en = 0;
		} while(cmd->noeat);

		/* Save character in buffer */
		if (buf_idx<23 && buf_en)
			buf[buf_idx++]=c;
		if (!buf_en)
			++ofst;
		buf[buf_idx] = 0;

		if(c=='\n')
			break;
	}
	/* Return new state number */
	h_state.state = h->no;
	return h_state;
}

/* Subroutines for load_dfa() */

static struct high_state *find_state(struct high_syntax *syntax,unsigned char *name)
{
	int x;
	struct high_state *state;

	/* Find state */
	for(x=0;x!=syntax->nstates;++x)
		if(!strcmp(syntax->states[x]->name,name))
			break;

	/* It doesn't exist, so create it */
	if(x==syntax->nstates) {
		int y;
		state=joe_malloc(sizeof(struct high_state));
		state->name=joe_strdup(name);
		state->no=syntax->nstates;
		state->color=FG_WHITE;
		if(!syntax->nstates)
			/* We're the first state */
			syntax->default_cmd.new_state = state;
		if(syntax->nstates==syntax->szstates)
			syntax->states=joe_realloc(syntax->states,sizeof(struct high_state *)*(syntax->szstates*=2));
		syntax->states[syntax->nstates++]=state;
		for(y=0; y!=256; ++y)
			state->cmd[y] = &syntax->default_cmd;
		state->delim = 0;
	} else
		state = syntax->states[x];
	return state;
}

/* Create empty command */

static void iz_cmd(struct high_cmd *cmd)
{
	cmd->noeat = 0;
	cmd->recolor = 0;
	cmd->start_buffering = 0;
	cmd->stop_buffering = 0;
	cmd->save_c = 0;
	cmd->save_s = 0;
	cmd->new_state = 0;
	cmd->keywords = 0;
	cmd->delim = 0;
	cmd->ignore = 0;
}

static struct high_cmd *mkcmd()
{
	struct high_cmd *cmd = joe_malloc(sizeof(struct high_cmd));
	iz_cmd(cmd);
	return cmd;
}

/* Globally defined colors */

struct high_color *global_colors;

struct high_color *find_color(struct high_color *colors,unsigned char *name,unsigned char *syn)
{
	unsigned char bf[256];
	struct high_color *color;
	joe_snprintf_2((char *)bf, sizeof(bf), "%s.%s", syn, name);
	for (color = colors; color; color = color->next)
		if (!strcmp(color->name,bf)) break;
	if (color)
		return color;
	for (color = colors; color; color = color->next)
		if (!strcmp(color->name,name)) break;
	return color;
}

void parse_color_def(struct high_color **color_list,unsigned char *p,unsigned char *name,int line)
{
	unsigned char bf[256];
	if(!parse_tows(&p, bf)) {
		struct high_color *color, *gcolor;

		/* Find color */
		color=find_color(*color_list,bf,name);

		/* If it doesn't exist, create it */
		if(!color) {
			color = joe_malloc(sizeof(struct high_color));
			color->name = joe_strdup(bf);
			color->color = 0;
			color->next = *color_list;
			*color_list = color;
		} else {
			fprintf(stderr,"%s %d: Class already defined\n",name,line);
		}

		/* Find it in global list */
		if (color_list != &global_colors && (gcolor=find_color(global_colors,bf,name))) {
			color->color = gcolor->color;
		} else {
			/* Parse color definition */
			while(parse_ws(&p,'#'), !parse_ident(&p,bf,255)) {
				color->color |= meta_color(bf);
			}
		}
	}
}

/* Load syntax file */

struct high_syntax *syntax_list;

struct high_syntax *load_dfa(unsigned char *name)
{
	unsigned char buf[1024];
	unsigned char bf[256];
	unsigned char bf1[256];
	int clist[256];
	unsigned char *p;
	int c;
	FILE *f;
	struct high_state *state=0;	/* Current state */
	struct high_syntax *syntax;	/* New syntax table */
	int line = 0;

	if (!name)
		return NULL;

	if(!attr_buf) {
		attr_size = 1024;
		attr_buf = joe_malloc(sizeof(int)*attr_size);
	}

	/* Find syntax table */

	/* Already loaded? */
	for(syntax=syntax_list;syntax;syntax=syntax->next)
		if(!strcmp(syntax->name,name))
			return syntax;

	/* Load it */
	p = (unsigned char *)getenv("HOME");
	if (p) {
		joe_snprintf_2((char *)buf,sizeof(buf),"%s/.joe/syntax/%s.jsf",p,name);
		f = fopen((char *)buf,"r");
	}

	if (!f) {
		joe_snprintf_2((char *)buf,sizeof(buf),"%ssyntax/%s.jsf",JOERC,name);
		f = fopen((char *)buf,"r");
	}
	if(!f)
		return 0;

	/* Create new one */
	syntax = joe_malloc(sizeof(struct high_syntax));
	syntax->name = joe_strdup(name);
	syntax->next = syntax_list;
	syntax_list = syntax;
	syntax->nstates = 0;
	syntax->color = 0;
	syntax->states = joe_malloc(sizeof(struct high_state *)*(syntax->szstates=64));
	syntax->sync_lines = 50;
	iz_cmd(&syntax->default_cmd);

	/* Parse file */
	while(fgets((char *)buf,1023,f)) {
		++line;
		p = buf;
		c = parse_ws(&p,'#');
		if(!parse_char(&p, ':')) {
			if(!parse_ident(&p, bf, 255)) {

				state = find_state(syntax,bf);

				parse_ws(&p,'#');
				if(!parse_ident(&p,bf,255)) {
					struct high_color *color;
					for(color=syntax->color;color;color=color->next)
						if(!strcmp(color->name,bf))
							break;
					if(color)
						state->color=color->color;
					else {
						state->color=0;
						fprintf(stderr,"%s %d: Unknown class\n",name,line);
					}
				} else
					fprintf(stderr,"%s %d: Missing color for state definition\n",name,line);
			} else
				fprintf(stderr,"%s %d: Missing state name\n",name,line);
		} else if(!parse_char(&p, '=')) {
			parse_color_def(&syntax->color,p,name,line);
		} else if(!parse_char(&p, '-')) { /* No. sync lines */
			if(parse_int(&p, &syntax->sync_lines))
				syntax->sync_lines = -1;
		} else {
			c = parse_ws(&p,'#');

			if (!c) {
			} else if (c=='"' || c=='*' || c=='&') {
				if (state) {
					struct high_cmd *cmd;
					int delim = 0;
					if(!parse_field(&p, US "*")) {
						int z;
						for(z=0;z!=256;++z)
							clist[z] = 1;
					} else if(!parse_field(&p, US "&")) {
						delim = 1;
					} else {
						c = parse_string(&p, bf, 255);
						if(c)
							fprintf(stderr,"%s %d: Bad string\n",name,line);
						else {
							int z;
							int first, second;
							unsigned char *t = bf;
							for(z=0;z!=256;++z)
								clist[z] = 0;
							while(!parse_range(&t, &first, &second)) {
								if(first>second)
									second = first;
								while(first<=second)
									clist[first++] = 1;
							}
						}
					}
					/* Create command */
					cmd = mkcmd();
					parse_ws(&p,'#');
					if(!parse_ident(&p,bf,255)) {
						int z;
						cmd->new_state = find_state(syntax,bf);

						/* Parse options */
						while (parse_ws(&p,'#'), !parse_ident(&p,bf,255))
							if(!strcmp(bf,"buffer")) {
								cmd->start_buffering = 1;
							} else if(!strcmp(bf,"hold")) {
								cmd->stop_buffering = 1;
							} else if(!strcmp(bf,"save_c")) {
								cmd->save_c = 1;
							} else if(!strcmp(bf,"save_s")) {
								cmd->save_s = 1;
							} else if(!strcmp(bf,"recolor")) {
								parse_ws(&p,'#');
								if(!parse_char(&p,'=')) {
									parse_ws(&p,'#');
									if(parse_int(&p,&cmd->recolor))
										fprintf(stderr,"%s %d: Missing value for option\n",name,line);
								} else
									fprintf(stderr,"%s %d: Missing value for option\n",name,line);
							} else if(!strcmp(bf,"strings") || !strcmp(bf,"istrings")) {
								if (bf[0]=='i')
									cmd->ignore = 1;
								while(fgets((char *)buf,1023,f)) {
									++line;
									p = buf;
									c = parse_ws(&p,'#');
									if (*p) {
										if(!parse_field(&p,US "done"))
											break;
										if(!parse_string(&p,bf,255)) {
											parse_ws(&p,'#');
											if (cmd->ignore)
												lowerize(bf);
											if(!parse_ident(&p,bf1,255)) {
												struct high_cmd *kw_cmd=mkcmd();
												kw_cmd->noeat=1;
												kw_cmd->new_state = find_state(syntax,bf1);
												if (!strcmp((char *)bf, "&")) {
													cmd->delim = kw_cmd;
												} else {
													if(!cmd->keywords)
														cmd->keywords = htmk(64);
														htadd(cmd->keywords,joe_strdup(bf),kw_cmd);
												}
												while (parse_ws(&p,'#'), !parse_ident(&p,bf,255))
													if(!strcmp(bf,"buffer")) {
														kw_cmd->start_buffering = 1;
													} else if(!strcmp(bf,"hold")) {
														kw_cmd->stop_buffering = 1;
													} else if(!strcmp(bf,"recolor")) {
														parse_ws(&p,'#');
														if(!parse_char(&p,'=')) {
															parse_ws(&p,'#');
															if(parse_int(&p,&kw_cmd->recolor))
																fprintf(stderr,"%s %d: Missing value for option\n",name,line);
														} else
															fprintf(stderr,"%s %d: Missing value for option\n",name,line);
													} else
														fprintf(stderr,"%s %d: Unknown option\n",name,line);
											} else
												fprintf(stderr,"%s %d: Missing state name\n",name,line);
										} else
											fprintf(stderr,"%s %d: Missing string\n",name,line);
									}
								}
							} else if(!strcmp(bf,"noeat")) {
								cmd->noeat = 1;
							} else
								fprintf(stderr,"%s %d: Unknown option\n",name,line);

						/* Install command */
						if (delim)
							state->delim = cmd;
						else for(z=0;z!=256;++z)
							if(clist[z])
								state->cmd[z]=cmd;
					} else
						fprintf(stderr,"%s %d: Missing jump\n",name,line);
				} else
					fprintf(stderr,"%s %d: No state\n",name,line);
			} else
				fprintf(stderr,"%s %d: Unknown character\n",name,line);
		}
	}

	fclose(f);

	return syntax;
}
