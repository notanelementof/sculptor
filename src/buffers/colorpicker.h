#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <stdint.h>

#include "../buffer.h"
#include "../term.h"

void open_colorpicker(buffers_t *b, bool set_active, uint32_t size, void (*cb)(void *, color_t), color_t default_color, void *data); /// Creates a colorpicker buffer.

#endif
