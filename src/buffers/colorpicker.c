#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "colorpicker.h"

typedef struct colorpicker {
	void *data; /// Data passed into the buffer
	color_t col; /// Current color
	uint8_t component; /// Selected component
	void (*cb)(void *, color_t); /// Callback
} colorpicker_t;

static bool colorpicker_key(buffer_t *this, int key) {
	colorpicker_t *cp = this->data;
	switch(key) {
		case 'r':
			cp->component = 0;
			return true;
		case 'g':
			cp->component = 1;
			return true;
		case 'b':
			cp->component = 2;
			return true;
		#define ADD(N) \
			switch(cp->component) { \
				case 0: \
					cp->col.r += N; \
					break; \
				case 1: \
					cp->col.g += N; \
					break; \
				case 2: \
					cp->col.b += N; \
					break; \
			}
		case 'i':
			ADD(1);
			return true;
		case 'd':
			ADD(-1);
			return true;
		case 'I':
			ADD(16);
			return true;
		case 'D':
			ADD(-16);
			return true;
		#undef ADD
		case KEY_ENTER:
			cp->cb(cp->data, cp->col);
			buffers_remove(this->parent, this->id);
			return true;
		case KEY_ESCAPE:
			buffers_remove(this->parent, this->id);
			return true;
	}
	return false;
}

void colorpicker_render(buffer_t *this) {
	colorpicker_t *cp = this->data;
	buffer_printf(this, 0, 0, cp->component == 0 ? COLOR_LIGHT_GREEN : COLOR_WHITE, COLOR_BLACK, "Red   (r) (i/I/d/D): %d", cp->col.r);
	buffer_printf(this, 0, 1, cp->component == 1 ? COLOR_LIGHT_GREEN : COLOR_WHITE, COLOR_BLACK, "Green (g) (i/I/d/D): %d", cp->col.g);
	buffer_printf(this, 0, 2, cp->component == 2 ? COLOR_LIGHT_GREEN : COLOR_WHITE, COLOR_BLACK, "Blue  (b) (i/I/d/D): %d", cp->col.b);
	buffer_printf(this, 0, 3, COLOR_WHITE, COLOR_BLACK, "Press enter to confirm, or escape to cancel.");
	for(size_t i = 0; i < this->width; i++)
		buffer_putch(this, i, 4, cp->col, cp->col, ' ');
}

void open_colorpicker(buffers_t *b, bool set_active, uint32_t size, void (*cb)(void *, color_t), color_t default_color, void *data) {
	colorpicker_t *cp = malloc(sizeof(colorpicker_t));
	*cp = (colorpicker_t){data, default_color, 0, cb};
	buffers_add(b, set_active, next_buffer_id(), cp, strdup("Color Picker"), size, &colorpicker_key, &colorpicker_render, NULL);
}
