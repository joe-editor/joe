/*
 * Example code to show how to copy the security context from one file to
 * another.
 */
#ifdef WITH_SELINUX
#include <selinux/selinux.h>
static int selinux_enabled = -1;
#endif
#include <errno.h>

int
copy_security_context(const char *from_file, const char *to_file)
{
#ifdef WITH_SELINUX
	if (selinux_enabled == -1)
		selinux_enabled = is_selinux_enabled();
	if (selinux_enabled) {
		security_context_t scontext;
		if (getfilecon(from_file, &scontext) < 0) {
			/*
			 * If the filesystem doesn't support extended
			 * attributes, the original had no special security
			 * context and the target cannot have one either.
			 */
			if (errno == EOPNOTSUPP)
				return 0;

			error(0, errno, "Could not get security context for %s",
			      from_file);
			return 1;
		}
		if (setfilecon(to_file, scontext) < 0) {
			error(0, errno, "Could not set security context for %s",
			      to_file);
			freecon(scontext);
			return 1;
		}
		freecon(scontext);
	}
#endif
	return 0;
}

int
match_default_security_context(const char *from_file)
{
#ifdef WITH_SELINUX
	if (selinux_enabled == -1)
		selinux_enabled = is_selinux_enabled();
	if (selinux_enabled) {
		security_context_t scontext;
		if (getfilecon(from_file, &scontext) < 0) {
			/*
			 * If the filesystem doesn't support extended
			 * attributes, the original had no special security
			 * context and the target cannot have one either.
			 */
			if (errno == EOPNOTSUPP)
				return 0;

			error(0, errno, "Could not get security context for %s",
			      from_file);
			return 1;
		}
		if (setfscreatecon(scontext) < 0) {
			error(0, errno,
			      "Could not set default security context for %s",
			      from_file);
			freecon(scontext);
			return 1;
		}
		freecon(scontext);
	}
#endif
	return 0;
}

int
reset_default_security_context()
{
#ifdef WITH_SELINUX
	if (selinux_enabled == -1)
		selinux_enabled = is_selinux_enabled();
	if (selinux_enabled) {
		if (setfscreatecon(0) < 0) {
			error(0, errno,
			      "Could not reset default security context");
			return 1;
		}
	}
#endif
	return 0;
}

int
output_security_context(char *from_file)
{
#ifdef WITH_SELINUX
	if (selinux_enabled == -1)
		selinux_enabled = is_selinux_enabled();
	if (selinux_enabled) {
		security_context_t scontext;
		if (getfilecon(from_file, &scontext) < 0) {
			/*
			 * If the filesystem doesn't support extended
			 * attributes, the original had no special security
			 * context and the target cannot have one either.
			 */
			if (errno == EOPNOTSUPP)
				return 0;

			error(0, errno, "Could not get security context for %s",
			      from_file);
			return 1;
		}
		error(0, 0, "%s Security Context %s", from_file, scontext);
		freecon(scontext);
	}
#endif
}

#if 0

/*
  Test program compile using the following command
  cc -o t t.c -DWITH_SELINUX -DTEST -lselinux
 */

#include <stdio.h>
#include <stdlib.h>
main(int argc, char **argv)
{

	printf("%d: %s\n", argc, argv[1]);
	if (argc == 3) {
		copy_security_context(argv[1], argv[2]);
		output_security_context(argv[2]);
	}
	if (argc == 2) {
		FILE *fd;
		char *temp;
		match_default_security_context(argv[1]);
		mkstemp(temp);
		printf("temp=%s", temp);
		fd = fopen(temp, "w");
		fclose(fd);
		output_security_context(temp);
		reset_default_security_context();
	}
}
#endif
