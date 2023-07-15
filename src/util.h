#ifndef UTIL_H
#define UTIL_H

// various random utilities that don't fit elsewhere

#include <stdint.h>
#include <stdbool.h>

char *validate_name(char *name); /// Validates a name. Returns NULL on success, an error message otherwise.
uint64_t millis(void); /// Gets the time in milliseconds
bool is_printable(char c); /// Returns true if a char is printable

#endif
