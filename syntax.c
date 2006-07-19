/*
 *	Syntax highlighting DFA interpreter
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

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
	int mark1 = 0;  /* offset to mark start from current pos */
	int mark2 = 0;  /* offset to mark end from current pos */
	int mark_en = 0;/* set if marking */

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
			if (cmd->delim && !zcmp(h_state.saved_s,buf)) {
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
				if (attr + x >= attr_buf)
					attr[x] = h -> color;

			/* Mark recoloring */
			if (cmd->recolor_mark)
				for(x= -mark1;x<-mark2;++x)
					attr[x] = h -> color;

			/* Save string? */
			if (cmd->save_s)
				zcpy(h_state.saved_s,buf);

			/* Save character? */
			if (cmd->save_c) {
				h_state.saved_s[1] = 0;
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
				
			/* Set mark begin? */
			if (cmd->start_mark)
			{
				mark2 = 1;
				mark1 = 1;
				mark_en = 1;
			}
				
			/* Set mark end? */
			if(cmd->stop_mark)
			{
				mark_en = 0;
				mark2 = 1;
			}
		} while(cmd->noeat);

		/* Save character in buffer */
		if (buf_idx<23 && buf_en)
			buf[buf_idx++]=c;
		if (!buf_en)
			++ofst;
		buf[buf_idx] = 0;
		
		/* Update mark pointers */
		++mark1;
		if(!mark_en)
			++mark2;

		if(c=='\n')
			break;
	}
	/* Return new state number */
	h_state.state = h->no;
	return h_state;
}

/* Subroutines for load_dfa() */

static struct high_state *find_state(struct high_syntax *syntax,unsigned char *prefix,unsigned char *name)
{
	unsigned char buf[256];
	struct high_state *state;

	joe_snprintf_2(buf, sizeof(buf), "%s%s", prefix, name);

	/* Find state */
	state = htfind(syntax->ht_states, buf);

	/* It doesn't exist, so create it */
	if(!state) {
		int y;
		state=joe_malloc(sizeof(struct high_state));
		state->name=zdup(buf);
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
		htadd(syntax->ht_states, state->name, state);
	}

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
	cmd->start_mark = 0;
	cmd->stop_mark = 0;
	cmd->recolor_mark = 0;
	cmd->call = 0;
	cmd->call_subr = 0;
	cmd->parms = 0;
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
	joe_snprintf_2(bf, sizeof(bf), "%s.%s", syn, name);
	for (color = colors; color; color = color->next)
		if (!zcmp(color->name,bf)) break;
	if (color)
		return color;
	for (color = colors; color; color = color->next)
		if (!zcmp(color->name,name)) break;
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
			color->name = zdup(bf);
			color->color = 0;
			color->next = *color_list;
			*color_list = color;
		} else {
			i_printf_2((char *)joe_gettext(_("%s %d: Class already defined\n")),name,line);
		}

		/* Find it in global list */
		if (color_list != &global_colors && (gcolor=find_color(global_colors,bf,name))) {
			color->color = gcolor->color;
		} else {
			/* Parse color definition */
			while(parse_ws(&p,'#'), !parse_ident(&p,bf,sizeof(bf))) {
				color->color |= meta_color(bf);
			}
		}
	}
}

/* Load syntax file */

struct high_syntax *syntax_list;

/* Dump sytnax file */

void dump_syntax(BW *bw)
{
	struct high_syntax *syntax;
	for (syntax = syntax_list; syntax; syntax = syntax->next) {
		int x;
		unsigned char buf[1024];
		joe_snprintf_2(buf, sizeof(buf), "Syntax name=%s, nstates=%d\n",syntax->name,syntax->nstates);
		binss(bw->cursor, buf);
		pnextl(bw->cursor);
		for(x=0;x!=syntax->nstates;++x) {
			int y;
			int f = -1;
			struct high_state *s = syntax->states[x];
			joe_snprintf_2(buf, sizeof(buf), "   state %s %x\n",s->name,s->color);
			binss(bw->cursor, buf);
			pnextl(bw->cursor);
			for (y = 0; y != 256; ++y) {
				if (f == -1)
					f = y;
				else if (s->cmd[f]->new_state != s->cmd[y]->new_state) {
					joe_snprintf_4(buf, sizeof(buf), "     [%d-%d] -> %s %d\n",f,y-1,s->cmd[f]->new_state->name,s->cmd[f]->recolor);
					binss(bw->cursor, buf);
					pnextl(bw->cursor);
					f = y;
				}
			}
			joe_snprintf_4(buf, sizeof(buf), "     [%d-%d] -> %s %d\n",f,y-1,s->cmd[f]->new_state->name,s->cmd[f]->recolor);
			binss(bw->cursor, buf);
			pnextl(bw->cursor);
		}
	}
}

struct ifstack {
	struct ifstack *next;
	int ignore;	/* Ignore input lines if set */
	int skip;	/* Set to skip the else part */
	int else_part;	/* Set if we're in the else part */
	int line;
};

struct syparm *parse_parms(unsigned char **ptr,unsigned char *name,int line)
{
	unsigned char *p = *ptr;
	unsigned char bf[256];
	struct syparm *sy = 0;

	parse_ws(&p, '#');
	if (!parse_char(&p, '(')) {
		for (;;) {
			parse_ws(&p, '#');
			if (!parse_char(&p, ')'))
				break;
			else if (!parse_ident(&p,bf,sizeof(bf))) {
				struct syparm *n = (struct syparm *)joe_malloc(sizeof(struct syparm));
				n->next = sy;
				sy = n;
				n->name = zdup(bf);
			} else {
				i_printf_2((char *)joe_gettext(_("%s %d: Missing )\n")),name,line);
				break;
			}
		}
	}

	*ptr = p;
	return sy;
}

struct high_state *append_dfa(struct high_syntax *syntax, unsigned char *prefix, unsigned char *name, struct high_state *rtn,
                              int *needs_link,struct syparm *parms,unsigned char *subr)
{
	unsigned char full_name[1024];
	unsigned char buf[1024];
	unsigned char bf[256];
	unsigned char bf1[256];
	int clist[256];
	unsigned char *p;
	int c;
	FILE *f = 0;
	struct ifstack *stack=0;
	struct high_state *state=0;	/* Current state */
	struct high_state *first=0;	/* First state */
	int line = 0;
	int this_one = 0;
	int inside_subr = 0;
	unsigned char *short_name = name;

	/* Load it */
	p = (unsigned char *)getenv("HOME");
	if (p) {
		joe_snprintf_2(full_name,sizeof(full_name),"%s/.joe/syntax/%s.jsf",p,name);
		f = fopen((char *)full_name,"r");
	}

	if (!f) {
		joe_snprintf_2(full_name,sizeof(full_name),"%ssyntax/%s.jsf",JOERC,name);
		f = fopen((char *)full_name,"r");
	}
	if(!f)
		return 0;
	name = full_name;

	/* Color are always file local */
	syntax->color = 0;

	/* Parse file */
	while(fgets((char *)buf,1023,f)) {
		++line;
		p = buf;
		c = parse_ws(&p,'#');
		if (!parse_char(&p, '.')) {
			if (!parse_ident(&p, bf, sizeof(bf))) {
				if (!zcmp(bf, USTR "ifdef")) {
					struct ifstack *st = (struct ifstack *)joe_malloc(sizeof(struct ifstack));
					st->next = stack;
					st->else_part = 0;
					st->ignore = 1;
					st->skip = 1;
					st->line = line;
					if (!stack || !stack->ignore) {
						parse_ws(&p,'#');
						if (!parse_ident(&p, bf, sizeof(bf))) {
							struct syparm *sy;
							for (sy = parms; sy; sy = sy->next)
								if (!zcmp(sy->name, bf))
									break;
							if (sy)
								st->ignore = 0;
							st->skip = 0;
						} else {
							i_printf_2((char *)joe_gettext(_("%s %d: missing parameter for ifdef\n")),name,line);
						}
					}
					stack = st;
				} else if (!zcmp(bf, USTR "else")) {
					if (stack && !stack->else_part) {
						stack->else_part = 1;
						if (!stack->skip)
							stack->ignore = !stack->ignore;
					} else
						i_printf_2((char *)joe_gettext(_("%s %d: else with no matching if\n")),name,line);
				} else if (!zcmp(bf, USTR "endif")) {
					if (stack) {
						struct ifstack *st = stack;
						stack = st->next;
						joe_free(st);
					} else
						i_printf_2((char *)joe_gettext(_("%s %d: endif with no matching if\n")),name,line);
				} else if (!zcmp(bf, USTR "subr")) {
					parse_ws(&p, '#');
					if (parse_ident(&p, bf, sizeof(bf))) {
						i_printf_2((char *)joe_gettext(_("%s %d: Missing subroutine name\n")),name,line);
					} else {
						if (!stack || !stack->ignore) {
							inside_subr = 1;
							this_one = 0;
							if (subr && !zcmp(bf, subr))
								this_one = 1;
						}
					}
				} else if (!zcmp(bf, USTR "end")) {
					if (!stack || !stack->ignore) {
						this_one = 0;
						inside_subr = 0;
					}
				} else {
					i_printf_2((char *)joe_gettext(_("%s %d: Unknown control statement\n")),name,line);
				}
			} else {
				i_printf_2((char *)joe_gettext(_("%s %d: Missing control statement name\n")),name,line);
			}
		} else if (stack && stack->ignore) {
			/* Ignore this line because of ifdef */
		} else if(!parse_char(&p, '=')) {
			/* Parse color */
			parse_color_def(&syntax->color,p,name,line);
		} else if ((subr && !this_one) || (!subr && inside_subr)) {
			/* Ignore this line because it's not the code we want */
		} else if(!parse_char(&p, ':')) {
			if(!parse_ident(&p, bf, sizeof(bf))) {

				state = find_state(syntax,prefix,bf);

				if (!first)
					first = state;

				parse_ws(&p,'#');
				if(!parse_ident(&p,bf,sizeof(bf))) {
					struct high_color *color;
					for(color=syntax->color;color;color=color->next)
						if(!zcmp(color->name,bf))
							break;
					if(color)
						state->color=color->color;
					else {
						state->color=0;
						i_printf_2((char *)joe_gettext(_("%s %d: Unknown class\n")),name,line);
					}
				} else
					i_printf_2((char *)joe_gettext(_("%s %d: Missing color for state definition\n")),name,line);
			} else
				i_printf_2((char *)joe_gettext(_("%s %d: Missing state name\n")),name,line);
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
					if(!parse_field(&p, USTR "*")) {
						int z;
						for(z=0;z!=256;++z)
							clist[z] = 1;
					} else if(!parse_field(&p, USTR "&")) {
						delim = 1;
					} else {
						c = parse_string(&p, bf, sizeof(bf));
						if(c < 0)
							i_printf_2((char *)joe_gettext(_("%s %d: Bad string\n")),name,line);
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
					if(!parse_ident(&p,bf,sizeof(bf))) {
						int z;
						cmd->new_state = find_state(syntax,prefix,bf);

						/* Parse options */
						while (parse_ws(&p,'#'), !parse_ident(&p,bf,sizeof(bf)))
							if(!zcmp(bf,USTR "buffer")) {
								cmd->start_buffering = 1;
							} else if(!zcmp(bf,USTR "hold")) {
								cmd->stop_buffering = 1;
							} else if(!zcmp(bf,USTR "return")) {
								if (rtn)
									cmd->new_state = rtn;
							} else if(!zcmp(bf,USTR "save_c")) {
								cmd->save_c = 1;
							} else if(!zcmp(bf,USTR "save_s")) {
								cmd->save_s = 1;
							} else if(!zcmp(bf,USTR "recolor")) {
								parse_ws(&p,'#');
								if(!parse_char(&p,'=')) {
									parse_ws(&p,'#');
									if(parse_int(&p,&cmd->recolor))
										i_printf_2((char *)joe_gettext(_("%s %d: Missing value for option\n")),name,line);
								} else
									i_printf_2((char *)joe_gettext(_("%s %d: Missing value for option\n")),name,line);
							} else if(!zcmp(bf,USTR "call")) {
								parse_ws(&p,'#');
								if(!parse_char(&p,'=')) {
									parse_ws(&p,'#');
									if (!parse_char(&p,'.')) {
										cmd->call = zdup(short_name);
										goto subr;
									} else if (parse_ident(&p,bf1,sizeof(bf1)))
										i_printf_2((char *)joe_gettext(_("%s %d: Missing value for option\n")),name,line);
									else {
										cmd->call = zdup(bf1);
										if (!parse_char(&p,'.')) {
											subr:
											if (parse_ident(&p,bf1,sizeof(bf1)))
												i_printf_2((char *)joe_gettext(_("%s %d: Missing subroutine name\n")),name,line);
											else
												cmd->call_subr = zdup(bf1);
										}
										*needs_link = 1;
										cmd->parms = parse_parms(&p,name,line);
									}
								} else
									i_printf_2((char *)joe_gettext(_("%s %d: Missing value for option\n")),name,line);
							} else if(!zcmp(bf,USTR "strings") || !zcmp(bf,USTR "istrings")) {
								if (bf[0]=='i')
									cmd->ignore = 1;
								while(fgets((char *)buf,1023,f)) {
									++line;
									p = buf;
									c = parse_ws(&p,'#');
									if (*p) {
										if(!parse_field(&p,USTR "done"))
											break;
										if(parse_string(&p,bf,sizeof(bf)) >= 0) {
											parse_ws(&p,'#');
											if (cmd->ignore)
												lowerize(bf);
											if(!parse_ident(&p,bf1,sizeof(bf1))) {
												struct high_cmd *kw_cmd=mkcmd();
												kw_cmd->noeat=1;
												kw_cmd->new_state = find_state(syntax,prefix,bf1);
												if (!zcmp(bf, USTR "&")) {
													cmd->delim = kw_cmd;
												} else {
													if(!cmd->keywords)
														cmd->keywords = htmk(64);
														htadd(cmd->keywords,zdup(bf),kw_cmd);
												}
												while (parse_ws(&p,'#'), !parse_ident(&p,bf,sizeof(bf)))
													if(!zcmp(bf,USTR "buffer")) {
														kw_cmd->start_buffering = 1;
													} else if(!zcmp(bf,USTR "hold")) {
														kw_cmd->stop_buffering = 1;
													} else if(!zcmp(bf,USTR "recolor")) {
														parse_ws(&p,'#');
														if(!parse_char(&p,'=')) {
															parse_ws(&p,'#');
															if(parse_int(&p,&kw_cmd->recolor))
																i_printf_2((char *)joe_gettext(_("%s %d: Missing value for option\n")),name,line);
														} else
															i_printf_2((char *)joe_gettext(_("%s %d: Missing value for option\n")),name,line);
													} else
														i_printf_2((char *)joe_gettext(_("%s %d: Unknown option\n")),name,line);
											} else
												i_printf_2((char *)joe_gettext(_("%s %d: Missing state name\n")),name,line);
										} else
											i_printf_2((char *)joe_gettext(_("%s %d: Missing string\n")),name,line);
									}
								}
							} else if(!zcmp(bf,USTR "noeat")) {
								cmd->noeat = 1;
							} else if(!zcmp(bf,USTR "mark")) {
								cmd->start_mark = 1;
							} else if(!zcmp(bf,USTR "markend")) {
								cmd->stop_mark = 1;
							} else if(!zcmp(bf,USTR "recolormark")) {
								cmd->recolor_mark = 1;
							} else
								i_printf_2((char *)joe_gettext(_("%s %d: Unknown option\n")),name,line);

						/* Install command */
						if (delim)
							state->delim = cmd;
						else for(z=0;z!=256;++z)
							if(clist[z])
								state->cmd[z]=cmd;
					} else
						i_printf_2((char *)joe_gettext(_("%s %d: Missing jump\n")),name,line);
				} else
					i_printf_2((char *)joe_gettext(_("%s %d: No state\n")),name,line);
			} else
				i_printf_2((char *)joe_gettext(_("%s %d: Unknown character\n")),name,line);
		}
	}

	while (stack) {
		struct ifstack *st = stack;
		stack = st->next;
		i_printf_2((char *)joe_gettext(_("%s %d: ifdef with no matching endif\n")),name,st->line);
		joe_free(st);
	}

	fclose(f);

	return first;
}

/* Load dfa */

void link_syntax(struct high_syntax *syntax);

struct high_syntax *load_dfa(unsigned char *name)
{
	struct high_syntax *syntax;	/* New syntax table */
	int needs_link = 0;

	if (!name)
		return NULL;

	if(!attr_buf) {
		attr_size = 1024;
		attr_buf = joe_malloc(sizeof(int)*attr_size);
	}

	/* Find syntax table */

	/* Already loaded? */
	for(syntax=syntax_list;syntax;syntax=syntax->next)
		if(!zcmp(syntax->name,name))
			return syntax;

	/* Create new one */
	syntax = joe_malloc(sizeof(struct high_syntax));
	syntax->name = zdup(name);
	syntax->next = syntax_list;
	syntax->nstates = 0;
	syntax->color = 0;
	syntax->states = joe_malloc(sizeof(struct high_state *)*(syntax->szstates = 64));
	syntax->ht_states = htmk(syntax->szstates);
	syntax->sync_lines = 50;
	syntax->recur = 0;
	iz_cmd(&syntax->default_cmd);

	if (append_dfa(syntax,USTR "",name,NULL,&needs_link,NULL,NULL)) {
		if (needs_link)
			link_syntax(syntax);
		/* dump_syntax(syntax); */
		syntax_list = syntax;
		return syntax;
	} else {
		htrm(syntax->ht_states);
		joe_free(syntax->name);
		joe_free(syntax->states);
		joe_free(syntax);
		return 0;
	}
}

/* Link in subroutine calls made to other tables */

/* List of existing subroutines */

struct sub_list {
	struct sub_list *next;
	unsigned char *name;
	struct high_state *rtn;	/* Return address */
	struct high_state *sub;	/* Pointer to subroutine */
};

struct high_state *find_sub(struct sub_list *list,unsigned char *name,struct high_state *rtn)
{
	/* printf("Find %s %p\n",name,(void *)rtn); */
	while (list) {
		if (!zcmp(list->name, name) && list->rtn==rtn)
			return list->rtn;
		list = list->next;
	}
	return 0;
}

struct sub_list *add_sub(struct sub_list *list,unsigned char *name,struct high_state *rtn,struct high_state *sub)
{
	struct sub_list *l=(struct sub_list *)malloc(sizeof(struct sub_list));
	l->next = list;
	l->name = zdup(name);
	l->rtn = rtn;
	l->sub = sub;
	/* printf("Add %p %s\n",(void *)rtn,name); */
	return l;
}

void link_syntax(struct high_syntax *syntax)
{
	int depth;
	int start = 0;
	struct sub_list *sub_list = 0;

	for (depth = 0; depth != 5; ++depth) {
		int inst = 0;
		int x = start;
		int nstates = syntax->nstates;
		start = nstates;
		for (; x != nstates; ++x) {
			struct high_state *state = syntax->states[x];
			int y;
			for(y = 0; y != 256; ++y) {
				struct high_cmd *cmd = state->cmd[y];
				if (cmd->call) {
					unsigned char buf1[256];
					unsigned char buf[256];
					struct high_state *sub;
					int needs_link = 0;
					joe_snprintf_3(buf1,sizeof(buf1),"%d.%s.%s",depth,cmd->call,cmd->call_subr);
					/* printf("%s is looking for %s.%s\n",state->name,cmd->call,(cmd->call_subr?cmd->call_subr:USTR "")); */
					if ( 1 ) { /* !(sub = find_sub(sub_list, buf1, cmd->new_state))) { */
						/* printf("loading...\n"); */
						joe_snprintf_2(buf,sizeof(buf),"%d.%d.",depth,inst++);
						sub = append_dfa(syntax,buf,cmd->call,cmd->new_state,&needs_link,cmd->parms,cmd->call_subr);
						if (sub)
							sub_list = add_sub(sub_list, buf1, cmd->new_state, sub);

					}
					if (sub)
						cmd->new_state = sub;
					cmd->call = 0;
				}
			}
		}
	}
}
