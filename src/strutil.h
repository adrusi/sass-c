#ifndef STRUTIL_H_
#define STRUTIL_H_

// Returns a pointer to the first character after the next newline in the
// provided string, or the next NUL, whichever occurs first.
extern char *str_next_line(char *str);

// Returns the number of characters until the next newline.
extern int str_line_len(char *str);

extern int str_has_prefix(char *str, char *pre);

extern int str_index_of_first_occurance(char *str, char *pat, int limit);

#endif
