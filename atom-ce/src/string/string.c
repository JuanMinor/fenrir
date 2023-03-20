//
//  string.c
//  Atom
//
//  Created by Juan Minor on 3/17/23.
//

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "include/string/string.h"

char *trim(char *string)
{
	if (!string)
		return NULL;
	if (!*string)
		return string;
	char *ptr;
	for (ptr = string + strlen(string) - 1; (ptr >= string) && isspace(*ptr); --ptr)
		;
	ptr[1] = '\0';
	return string;
}
