#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 1024   /* max bytes in pathname */

char *pipe_from(void)
{
        char *buffer = NULL;
		FILE *f = popen("fc-list | grep -F `fc-match mono | awk '{ print($1) }'` "
				"| awk 'BEGIN { FS = \":\" } ; { print($1) }'", "r");

		if (f) {
			buffer = (char *) malloc(PATH_MAX + 1);
			buffer[fread(buffer, 1, PATH_MAX, f)] = 0;
			pclose(f);
			char *newline = strchr(buffer, '\n');
			if (newline) *newline = 0;
		}
        return buffer;
}

