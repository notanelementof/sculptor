#include <stdio.h>

#include "term.h"

int main(void) {
	init_termios();
	terminal_t *term = term_new();
	while(true) {
		if(kbhit())
			break;
		term_clear(term, COLOR_BLACK);
		term_putch(term, 5, 5, COLOR_RED, COLOR_LIGHT_YELLOW, 'a');
		term_flush(term);
	}
}
