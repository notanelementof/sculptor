
#include <string.h>
#include <sys/time.h>

#include "util.h"

char *validate_name(char *name) {
	size_t namelen = strlen(name);
	if(namelen == 0)
		return "Name must be at least 1 character long.";
	for(size_t i = 0; i < namelen; i++) {
		char c = name[i];
		if((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '-' && c != '_')
			return "Allowed characters in names are letters, numbers, dash, and underscore.";
	}
	return NULL;
}

uint64_t millis(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

bool is_printable(char c) {
	return c >= 0x20 && c <= 0x7E;
}
