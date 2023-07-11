#include <stdio.h>
#include <string.h>

#include "term.h"
#include "buffer.h"
#include "buffers/menu.h"
#include "project.h"
#include "main.h"

static bool do_quit = false;

int main(void) {
	init_termios();
	hide_cursor();
	project_init();
	terminal_t *term = term_new();
	buffers_t *buffers = buffers_new(term);
	open_main_menu(buffers);
	while(true) {
		if(do_quit)
			break;
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
	term_clear(term, COLOR_BLACK);
	term_flush(term);
	buffers_free(buffers);
	term_free(term);
	show_cursor();
}

static void quit_cb(menu_item_t *this, void *data) {
	do_quit = true;
}

static void projects_cb(menu_item_t *this, void *data) {
	open_projects_menu((buffers_t *)data);
}

void open_main_menu(buffers_t *b) {
	buffers_clear(b);
	menu_info_t m = menu_create(b, true, "Main Menu", 5, 4,
		menu_label("Sculptor V" SCULPTOR_VERSION),
		menu_label("< - >"),
		menu_button("Projects", &projects_cb, b),
		menu_button("Quit", &quit_cb, NULL)
	);
	m.menu->selected = 2;
}

void quit(void) {
	do_quit = true;
}
