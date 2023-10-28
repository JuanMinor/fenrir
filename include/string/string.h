//
//  string.h
//  @loki
//
//  Created by Juan Minor on 3/19/23.
//

#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <string.h>

#include "include/core/core.h"

char *trim(char *__string);

char **split(const char *__string, const char *__delimiters, size_t *__size);

#endif
