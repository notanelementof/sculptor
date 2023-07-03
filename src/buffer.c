#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "buffer.h"

buffers_t *buffers_new(terminal_t *term) {
	buffers_t *bp = malloc(sizeof(buffers_t));
	*bp = (buffers_t){0, 0, NULL, term};
	return bp;
}

void buffers_free(buffers_t *b) {
	for(size_t i = 0; i < b->len; i++)
		buffer_free(&b->items[i]);
	free(b->items);
	free(b);
}

void buffers_add(buffers_t *b, uint64_t id, void *data, char *name, uint32_t size, bool (*key)(buffer_t *, int), void (*render)(buffer_t *), void (*free)(buffer_t *)) {
	b->items = realloc(b->items, sizeof(buffer_t)*(++b->len));
	b->items[b->len-1] = (buffer_t){id, data, name, b, size, 0, 0, key, render, free};
}

void buffers_remove(buffers_t *b, uint64_t id) {
	size_t idx = 0;
	for(size_t i = 0; i < b->len; i++) {
		if(b->items[i].id == id) {
			idx = i;
			goto found;
		}
	}
	return; // buffer was not found
	found:
	buffer_free(&b->items[idx]);
	if(b->len == 1) {
		// realloc can't work with new_size of 0, so here we just free items
		free(b->items);
		b->len = 0;
		return;
	}
	// shift all elements after idx
	memmove(&b->items[idx], &b->items[idx+1], b->len-idx);
	b->items = realloc(b->items, (--b->len)*sizeof(buffer_t));
}

void buffers_render(buffers_t *b) {
	uint32_t sum = 0;
	for(size_t i = 0; i < b->len; i++) {
		sum += b->items[i].size;
	}
	uint32_t width_sum = 0;
	for(size_t i = 0; i < b->len; i++) {
		buffer_t *buf = &b->items[i];
		buf->width = (uint32_t)((buf->size/(double)sum)*(double)b->term->width);
		buf->x = width_sum;
		width_sum += buf->width;
		buffer_render(buf, i == b->active);
	}
}

bool buffers_key(buffers_t *b, int key) {
	buffer_t *active = &b->items[b->active];
	if(active->key != NULL && active->key(active, key))
		return true;
	switch(key) {
		case '1':
			if(b->active != 0)
				b->active--;
			else
				b->active = b->len-1;
			return true;
		case '2':
			if(b->active+1 < b->len)
				b->active++;
			else
				b->active = 0;
			return true;
	}
	return false;
}

uint64_t next_buffer_id(void) {
	static uint64_t id = 0;
	return id++;
}

void buffer_free(buffer_t *buf) {
	if(buf->free != NULL)
		buf->free(buf);
	free(buf->name);
	free(buf->data);
}

void buffer_render(buffer_t *buf, bool active) {
	color_t bg = active ? COLOR_LIGHT_GREEN : COLOR_GREEN;
	for(uint32_t i = 0; i < buf->width; i++)
		buffer_putch(buf, i, -1, COLOR_WHITE, bg, ' ');
	buffer_printf(buf, 0, -1, COLOR_BLACK, TRANSPARENT, "%s", buf->name);
	if(buf->render != NULL)
		buf->render(buf);
}

void buffer_putch(buffer_t *buf, size_t x, size_t y, color_t fg, color_t bg, char ch) {
	if(x >= buf->width)
		return; // out of bounds
	term_putch(buf->parent->term, x+buf->x, y+1, fg, bg, ch);
}

void buffer_printf(buffer_t *buf, size_t x, size_t y, color_t fg, color_t bg, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	
	size_t len = buf->width*buf->parent->term->height+1;
	char str[len];
	vsnprintf(str, len, fmt, ap);

	va_end(ap);

	size_t ox = x; // original x
	for(size_t i = 0; i < strlen(str); i++) {
		char c = str[i];
		switch(c) {
			case '\r':
				x = ox;
				break;
			case '\n':
				x = ox;
				y++;
				break;
			default:
				buffer_putch(buf, x, y, fg, bg, c);
				break;
		}
		x++;
	}
}
