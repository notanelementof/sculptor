#ifndef UTIL_H
#define UTIL_H

// various random utilities that don't fit elsewhere

#include <stdint.h>

char *validate_name(char *name); /// Validates a name. Returns NULL on success, an error message otherwise.
uint64_t millis(void); /// Gets the time in milliseconds

#endif
