#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "term.h"

typedef struct buffers buffers_t;
typedef struct buffer buffer_t;

/**
	Represents a buffer.

	A buffer is a part of the screen that can contain contents, and they are arranged horizontally.
*/
typedef struct buffer {
	uint64_t id; /// ID of this buffer. used to compare buffers.
	void *data; /// any data
	char *name; /// Name of the buffer (should be heap-allocated)
	buffers_t *parent; /// The parent buffers_t
	uint32_t size; /// The size of the buffer horizontally. The way this works is that the % of the total size is multiplied by the termiunal wuwidth to get the actual size.
	uint32_t x, width, height; /// X position, width and height of the buffer. calculated eachf rame before render() is called
	bool (*key)(buffer_t *this, int key); /// Called when a key is pressed while a buffer is active. should return true when the key input is used.
	void (*render)(buffer_t *this); /// Called each frame
	void (*free)(buffer_t *this); /// Function called to do cleanup. Should not do `free(this->data)`, as that is done automatically after this function finishes.
} buffer_t;

/// Represents a group of buffers
typedef struct buffers {
	size_t active; /// The active buffer
	size_t len;
	buffer_t *items;
	terminal_t *term;
} buffers_t;

buffers_t *buffers_new(terminal_t *term); /// Creates a new buffer group
void buffers_free(buffers_t *b); /// Free a buffer group (and all its children)
buffer_t *buffers_add(buffers_t *b, bool set_active, uint64_t id, void *data, char *name, uint32_t size, bool (*key)(buffer_t *, int), void (*render)(buffer_t *), void (*free)(buffer_t *)); /// Add a buffer. Returns a pointer to the created buffer
void buffers_remove(buffers_t *b, uint64_t id); /// Remove a buffer from a group
void buffers_render(buffers_t *b); /// Renders all buffers
bool buffers_key(buffers_t *b, int key); /// Tells a buffer group that a keyboard input has occured. Returns true if the key was consumed by the active buffer. Also handles the `1` and `2` keys for switching the active buffer
void buffers_clear(buffers_t *b); /// Delete all buffers within a buffers group

uint64_t next_buffer_id(void); /// Gives a new unique buffer ID

void buffer_free(buffer_t *buf); /// Frees a buffer. usually should not be called on its own.
void buffer_render(buffer_t *buf, bool active); /// Renders a buffer. usually should not be called on its own.
void buffer_putch(buffer_t *buf, size_t x, size_t y, color_t fg, color_t bg, char ch); /// Puts a character at a given position and color in a buffer
void buffer_printf(buffer_t *buf, size_t x, size_t y, color_t fg, color_t bg, const char *fmt, ...); /// printf's into a buffer

#endif
