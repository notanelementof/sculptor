#include <stdio.h>
#include <string.h>

#include "term.h"
#include "buffer.h"
#include "buffers/menu.h"

static void test_action(menu_item_t *this, void *data) {
	buffers_add((buffers_t *)data, true, next_buffer_id(), NULL, strdup(menu_get_textbox_value(this->parent, "buf-name")), (uint32_t)menu_get_numbox_value(this->parent, "buf-size"), NULL, NULL, NULL);
}

int main(void) {
	init_termios();
	hide_cursor();
	terminal_t *term = term_new();
	buffers_t *buffers = buffers_new(term);
	menu_create(buffers, true, "Main Menu", 5, 3,
		menu_textbox("Buffer Name", "test buffer", "buf-name", 64),
		menu_numbox("Buffer Size", 1, 1, 20, 1, "buf-size"),
		menu_button("Add Buffer", &test_action, buffers)
	);
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
