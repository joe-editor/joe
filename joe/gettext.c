/* JOE's gettext() library.  Why?  Once again we can not rely on the
 * system's or GNU's gettext being installed properly */

/* One modification from standard gettext: comments are allowed at
 * the start of strings: "|comment|comment|comment|foo".  The leading
 * '|' is required to indicate the comment.
 *
 * Comments can be used to make two otherwise identical strings distinct.
 */

#include "types.h"

HASH *gettext_ht;

static const char *ignore_prefix(const char *set)
{
	const char *s = zrchr(set, '|');
	if (s)
		++s;
	else
		s = set;
	return s;
}

const char *my_gettext(const char *s)
{
	if (gettext_ht) {
		const char *r = (const char *)htfind(gettext_ht, s);
		if (r)
			s = r;
	}
	if (s[0] == '|')
		return ignore_prefix(s);
	else
		return s;
}

static int load_po(FILE *f)
{
	char *buf = 0;
	char *msgid = vsdupz("");
	char *msgstr = vsdupz("");
	struct charmap *po_map = locale_map;
	int preload_flag = 0;

	while (preload_flag || vsgets(isfree(&buf), f)) {
		const char *p;
		preload_flag = 0;
		p = buf;
		parse_ws(&p, '#');
		if (!parse_field(&p, "msgid")) {
			ptrdiff_t ofst = 0;
			ptrdiff_t len;
			char *bf = 0;
			msgid = vscpyz(msgid, "");
			parse_ws(&p, '#');
			while ((len = parse_string(&p, &bf)) >= 0) {
				msgid = vscat(msgid, sv(bf));
				preload_flag = 0;
				ofst += len;
				parse_ws(&p, '#');
				if (!*p) {
					if (vsgets(&buf, f)) {
						p = buf;
						preload_flag = 1;
						parse_ws(&p, '#');
					} else {
						goto bye;
					}
				}
			}
		} else if (!parse_field(&p, "msgstr")) {
			ptrdiff_t ofst = 0;
			ptrdiff_t len;
			char *bf = 0;
			msgstr = vscpyz(msgstr, "");
			parse_ws(&p, '#');
			while ((len = parse_string(&p, &bf)) >= 0) {
				msgstr = vscat(msgstr, sv(bf));
				preload_flag = 0;
				ofst += len;
				parse_ws(&p, '#');
				if (!*p) {
					if (vsgets(&buf, f)) {
						p = buf;
						preload_flag = 1;
						parse_ws(&p, '#');
					} else {
						break;
					}
				}
			}
			if (msgid[0] && msgstr[0]) {
				/* Convert to locale character map */
				char *bf = my_iconv(NULL,locale_map,msgstr,po_map);
				/* Add to hash table */
				htadd(gettext_ht, zdup(msgid), zdup(bf));
			} else if (!msgid[0] && msgstr[0]) {
				char *tp = zstr(msgstr, "charset=");
				msgid = vscpyz(msgid, msgstr); /* Make sure msgid is long enough */
				msgid = vscpyz(msgid, ""); /* Truncate it */
				if (tp) {
					/* Copy character set name up to next delimiter */
					int x;
					tp += SIZEOF("charset=") - 1;
					while (*tp == ' ' || *tp == '\t') ++tp;
					for (x = 0; tp[x] && tp[x] !='\n' && tp[x] != '\r' && tp[x] != ' ' &&
					            tp[x] != '\t' && tp[x] != ';' && tp[x] != ','; ++x)
					            msgid[x] = tp[x];
					msgid[x] = 0;
					po_map = find_charmap(msgid);
					if (!po_map)
						po_map = locale_map;
				}
			}
		}
	}
	bye:
	fclose(f);
	obj_free(msgid);
	return 0;
}

/* Initialize my_gettext().  Call after locale_map has been set. */

void init_gettext(const char *s)
{
	FILE *f;
	char *buf = vsfmt(NULL,0, "%slang/%s.po",JOEDATA,s);
	if ((f = fopen(buf, "r"))) {
		/* Try specific language, like en_GB */
		gettext_ht = htmk(256);
		load_po(f);
	} else if (s[0] && s[1]) {
		/* Try generic language, like en */
		char *bf = vsfmt(NULL,0,"%slang/%c%c.po",JOEDATA,s[0],s[1]);
		if ((f = fopen(bf, "r"))) {
			gettext_ht = htmk(256);
			load_po(f);
		}
	}
	obj_free(buf);
}
