#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "term.h"

color_t color(uint8_t r, uint8_t g, uint8_t b) {
	return (color_t){r, g, b, false};
}

static struct termios prev_termios;

void init_termios(void) {
	struct termios new;

	tcgetattr(0, &new);
	prev_termios = new;

	cfmakeraw(&new);
	new.c_lflag &= ~ECHO;
	
	tcsetattr(0, TCSANOW, &new);
	
	atexit(&reset_termios);
}

void reset_termios(void) {
	tcsetattr(0, TCSANOW, &prev_termios);
}

int getch(void) {
	int r;
	unsigned char c;
	if((r = read(0, &c, sizeof(c))) < 0)
		return r;
	return c;
}

bool kbhit(void) {
	struct timeval tv = {0L, 0L};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv) > 0;
}

termsize_t get_term_size(void) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return (termsize_t){w.ws_col, w.ws_row};
}

// terminal

terminal_t *term_new(void) {
	terminal_t *term = malloc(sizeof(terminal_t));
	*term = (terminal_t){0, 0, NULL, NULL};
	term_resize(term, get_term_size());
	return term;
}

void term_resize(terminal_t *term, termsize_t ts) {
	// don't resize if size is the same
	if(term->width == ts.width && term->height == ts.height)
		return;
	free(term->curr);
	free(term->prev);
	term->width = ts.width;
	term->height = ts.height;
	size_t size = ts.width*ts.height*sizeof(tchar_t);
	term->curr = malloc(size);
	for(size_t i = 0; i < ts.width*ts.height; i++)
		term->curr[i] = (tchar_t){' ', 255, 255, 255, 0, 0, 0};
	term->prev = malloc(size);
	// set term->prev different so it always flushes after resize
	for(size_t i = 0; i < ts.width*ts.height; i++)
		term->curr[i] = (tchar_t){'!', 255, 255, 255, 0, 0, 0};
}

void term_free(terminal_t *term) {
	free(term->curr);
	free(term->prev);
	free(term);
}

void term_flush(terminal_t *term) {
	for(size_t i = 0; i < term->height; i++) {
		tchar_t *row = &term->curr[i*term->width];
		// skip this row if it's not different
		if(memcmp(row, &term->prev[i*term->width], term->width*sizeof(tchar_t)) == 0)
			continue;
		printf("\033[%zu;0H", i); // move to row
		// print each char
		for(size_t i = 0; i < term->width; i++) {
			tchar_t *tc = &row[i];
			printf("\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm%c", tc->fr, tc->fg, tc->fb, tc->br, tc->bg, tc->bb, tc->ch);
		}
	}
	// copy curr to prev
	memcpy(term->prev, term->curr, term->width*term->height*sizeof(tchar_t));
}

void term_putch(terminal_t *term, size_t x, size_t y, color_t fg, color_t bg, char ch) {
	tchar_t *tc = &term->curr[y*term->width+x];
	tc->ch = ch;
	if(!fg.transparent) {
		tc->fr = fg.r;
		tc->fg = fg.g;
		tc->fb = fg.b;
	}
	if(!bg.transparent) {
		tc->br = bg.r;
		tc->bg = bg.g;
		tc->bb = bg.b;
	}
}

void term_clear(terminal_t *term, color_t col) {
	for(size_t i = 0; i < term->width*term->height; i++)
		term->curr[i] = (tchar_t){' ', col.r, col.g, col.b, col.r, col.g, col.b};
}
