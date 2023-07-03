#include <stdio.h>
#include <string.h>

#include "term.h"
#include "buffer.h"

static void render(buffer_t *this) {
	buffer_printf(this, 0, 0, COLOR_LIGHT_RED, COLOR_BLACK, "%u %u", this->width, this->x);
}

int main(void) {
	init_termios();
	hide_cursor();
	terminal_t *term = term_new();
	buffers_t *buffers = buffers_new(term);
	buffers_add(buffers, next_buffer_id(), NULL, strdup("test"), 5, NULL, &render, NULL);
	buffers_add(buffers, next_buffer_id(), NULL, strdup("test2"), 3, NULL, &render, NULL);
	buffers_add(buffers, next_buffer_id(), NULL, strdup("test2"), 3, NULL, &render, NULL);
	while(true) {
		if(kbhit()) {
			char c = getch();
			if(!buffers_key(buffers, c)) {
				switch(c) {
					case 'q':
						goto exit;
				}
			}
		}
		term_clear(term, COLOR_BLACK);
		buffers_render(buffers);
		term_flush(term);
		term_resize(term, get_term_size());
	}
	exit:
	buffers_free(buffers);
	term_free(term);
	show_cursor();
}
