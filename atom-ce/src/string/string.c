//
//  string.c
//  @atom-ce
//
//  Created by Juan Minor on 3/17/23.
//

#include "include/string/string.h"

char *trim(char *__string)
{
	if (!__string)
		return NULL;
	if (!*__string)
		return __string;

	char *ptr;
	size_t length = strlen(__string);

	// Remove trailing whitespace
	for (ptr = __string + length - 1; (ptr >= __string) && isspace(*ptr); --ptr)
		;
	ptr[1] = '\0';

	// Remove leading whitespace
	for (ptr = __string; *ptr && isspace(*ptr); ++ptr)
		;

	// Move trimmed string to beginning of memory block
	if (ptr != __string)
		memmove(__string, ptr, strlen(ptr) + 1);

	return __string;
}

char **split(const char *__string, const char *__delimiters, size_t *__size)
{
	if (!__string || !__delimiters)
		return NULL;

	size_t length = strlen(__string);

	// Allocate memory for the initial size of the array of pointers to substrings.
	int capacity = 10;
	char **substrings = malloc(capacity * sizeof(char *));
	*__size = 0;

	const char *start = __string;
	const char *end;

	while ((end = strpbrk(start, __delimiters)) != NULL)
	{
		// Reallocate memory if needed.
		if (*__size == capacity)
		{
			capacity *= 2;
			substrings = realloc(substrings, capacity * sizeof(char *));
		}

		// Allocate memory for the current substring and copy the characters.
		int substring_length = end - start;
		char *substring = malloc(substring_length + 1);
		memcpy(substring, start, substring_length);
		substring[substring_length] = '\0';
		substrings[*__size] = substring;

		// Increment the size and move to the next substring.
		(*__size)++;
		start = end + 1;
	}

	// Add the last substring if it's not empty.
	if (*start != '\0')
	{
		// Reallocate memory if needed.
		if (*__size == capacity)
		{
			capacity += 1;
			substrings = realloc(substrings, capacity * sizeof(char *));
		}

		// Allocate memory for the last substring and copy the characters.
		char *substring = strdup(start);
		substrings[*__size] = substring;

		// Increment the size.
		(*__size)++;
	}

	// Reallocate the memory to the exact size needed.
	substrings = realloc(substrings, *__size * sizeof(char *));

	return substrings;
}