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
#include "syntax.h"

/* Parse one line.  Returns new state.
   'line' is advanced to start of next line.
   Array 'attr' has coloring for each character of line.
*/

int *attr_buf = 0;
int attr_size = 0;

int parse(struct high_syntax *syntax,P *line,int state)
{
	struct high_state *h = syntax->states[state];
			/* Current state */
	char buf[20];	/* Name buffer (trunc after 19 characters) */
	int buf_idx=0;	/* Index into buffer */
	int c;		/* Current character */
	int *attr_end = attr_buf+attr_size;
	int *attr = attr_buf;

	/* Get next character */
	while((c=pgetc(line))!=NO_MORE_DATA) {
		struct high_cmd *cmd;
		struct high_keyword *word;
		struct high_state *old_state;
		int x;

		if(attr==attr_end) {
			attr_buf = realloc(attr_buf,sizeof(int)*(attr_size*2));
			attr = attr_buf + attr_size;
			attr_size *= 2;
			attr_end = attr_buf + attr_size;
		}

		/* Color with current state */
		*attr++ = h->color;

		do {
			old_state = h;
			/* Get command for this character */
			cmd = h->cmd[c];
			/* Determine new state */
			for(word=cmd->keyword_list;word;word=word->next)
				if(!strcmp(word->name,buf)) break;
			if(word) {
				h = word->new_state;
				/* Recolor keyword */
				for(x= -(buf_idx+1);x<-1;++x)
					attr[x] = h -> color;
			} else {
				h = cmd->new_state;
				/* Recolor if necessary */
				for(x=cmd->recolor;x<0;++x)
					attr[x] = h -> color;
			}
			/* Start buffering? */
			if(cmd->start_buffering)
				buf_idx = 0;
		} while(cmd->noeat);

		/* Save in buffer */
		if(buf_idx<19) buf[buf_idx++]=c;
		buf[buf_idx] = 0;

		if(c=='\n')
			break;
	}
	/* Return new state number */
	return h->no;
}

/* Load syntax file */

struct high_syntax *syntax_list;

struct high_syntax *load_dfa(char *name)
{
	char buf[1024];
	char bf[256];
	int clist[256];
	char *p;
	int c;
	FILE *f;
	struct high_state *state=0;	/* Current state */
	struct high_syntax *syntax;	/* New syntax table */
	int szstates;			/* Malloc size of states table (in syntax) */
	int line = 0;

	if(!attr_buf) {
		attr_size = 1024;
		attr_buf = malloc(sizeof(int)*attr_size);
	}

	/* Find syntax table */

	/* Already loaded? */
	for(syntax=syntax_list;syntax;syntax=syntax->next)
		if(!strcmp(syntax->name,name))
			break;
	if(syntax)
		return syntax;

	/* Load it */
	sprintf(buf,"%s%s.jsf",JOERC,name);
	f=fopen(buf,"r");
	if(!f)
		return 0;

	/* Create new one */
	syntax = malloc(sizeof(struct high_syntax));
	syntax->name = strdup(name);
	syntax->next = syntax_list;
	syntax_list = syntax;
	syntax->nstates = 0;
	syntax->color = 0;
	syntax->states = malloc(sizeof(struct high_state *)*(szstates=64));

	/* Parse file */
	while(fgets(buf,1023,f)) {
		++line;
		p = buf;
		c = parse_ws(&p);
		if(!parse_char(&p, ':')) {
			if(!parse_ident(&p, bf, 255)) {

				int x;

				/* Find state */
				for(x=0;x!=syntax->nstates;++x)
					if(!strcmp(syntax->states[x]->name,bf))
						break;

				/* It doesn't exist, so create it */
				if(x==syntax->nstates) {
					state=malloc(sizeof(struct high_state));
					state->name=strdup(bf);
					state->no=syntax->nstates;
					state->color=FG_WHITE;
					/* FIXME: init cmd table? */
					if(syntax->nstates==szstates)
						syntax->states=realloc(syntax->states,sizeof(struct high_state *)*(szstates*=2));
					syntax->states[syntax->nstates++]=state;
				} else
					state = syntax->states[x];

				parse_ws(&p);
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
			if(!parse_ident(&p, bf, 255)) {
				struct high_color *color;

				/* Find color */
				for(color=syntax->color;color;color=color->next)
					if(!strcmp(color->name,bf))
						break;
				/* If it doesn't exist, create it */
				if(!color) {
					color = malloc(sizeof(struct high_color));
					color->name = strdup(bf);
					color->color = 0;
					color->next = syntax->color;
					syntax->color = color;
				} else {
					fprintf(stderr,"%s %d: Class already defined\n",name,line);
				}

				/* Parse color definition */
				while(parse_ws(&p), !parse_ident(&p,bf,255)) {
					color->color |= meta_color(bf);
				}
			}
		} else {
			c = parse_ws(&p);

			if (!c) {
			} else if (c=='"' || c=='*') {
				if (state) {
					struct high_cmd *cmd;
					if(!parse_field(&p, "*")) {
						int z;
						for(z=0;z!=256;++z)
							clist[z] = 1;
					} else {
						c = parse_string(&p, bf, 255);
						if(c)
							fprintf(stderr,"%s %d: Bad string\n",name,line);
						else {
							int z;
							int first, second;
							char *t = bf;
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
					cmd = malloc(sizeof(struct high_cmd));
					cmd->noeat = 0;
					cmd->recolor = 0;
					cmd->start_buffering = 0;
					cmd->new_state = 0;
					cmd->keyword_list = 0;

					parse_ws(&p);
					if(!parse_ident(&p,bf,255)) {
						struct high_state *h;
						int z;
						for(z=0;z!=syntax->nstates;++z)
							if(!strcmp(syntax->states[z]->name,bf))
								break;

						if(z==syntax->nstates) {
							h=malloc(sizeof(struct high_state));
							h->name=strdup(bf);
							h->no=syntax->nstates;
							h->color=FG_WHITE;
							/* FIXME: init cmd table? */
							if(syntax->nstates==szstates)
								syntax->states=realloc(syntax->states,sizeof(struct high_state *)*(szstates*=2));
							syntax->states[syntax->nstates++]=h;
						} else
							h = syntax->states[z];

						cmd->new_state = h;

						/* Parse options */
						while (parse_ws(&p), !parse_ident(&p,bf,255))
							if(!strcmp(bf,"buffer")) {
								cmd->start_buffering = 1;
							} else if(!strcmp(bf,"recolor")) {
								parse_ws(&p);
								if(!parse_char(&p,'=')) {
									parse_ws(&p);
									if(parse_int(&p,&cmd->recolor))
										fprintf(stderr,"%s %d: Missing value for option\n",name,line);
								} else
									fprintf(stderr,"%s %d: Missing value for option\n",name,line);
							} else if(!strcmp(bf,"strings")) {
								while(fgets(buf,1023,f)) {
									++line;
									p = buf;
									c = parse_ws(&p);
									if(!parse_field(&p,"done"))
										break;
									if(!parse_string(&p,bf,255)) {
										struct high_keyword *k=malloc(sizeof(struct high_keyword));
										k->name=strdup(bf);
										k->next=cmd->keyword_list;
										k->new_state=0;
										cmd->keyword_list=k;
										parse_ws(&p);
										if(!parse_ident(&p,bf,255)) {
											struct high_state *h;
											int z;
											for(z=0;z!=syntax->nstates;++z)
												if(!strcmp(syntax->states[z]->name,bf))
													break;

											if(z==syntax->nstates) {
												h=malloc(sizeof(struct high_state));
												h->name=strdup(bf);
												h->no=syntax->nstates;
												h->color=FG_WHITE;
												/* FIXME: init cmd table? */
												if(syntax->nstates==szstates)
													syntax->states=realloc(syntax->states,sizeof(struct high_state *)*(szstates*=2));
												syntax->states[syntax->nstates++]=h;
											} else
												h = syntax->states[z];

											k->new_state = h;
											
										} else
											fprintf(stderr,"%s %d: Missing state name\n",name,line);
									} else
										fprintf(stderr,"%s %d: Missing string\n",name,line);
								}
							} else if(!strcmp(bf,"noeat")) {
								cmd->noeat = 1;
							} else
								fprintf(stderr,"%s %d: Unknown option\n",name,line);

						/* Install command */
						for(z=0;z!=256;++z)
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
