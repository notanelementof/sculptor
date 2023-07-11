/// For creation and manipulation of scene buffers (including the events buffer)
#ifndef SCENE_BUFFER_H
#define SCENE_BUFFER_H

#include "../buffer.h"

void open_scene_buffer(buffers_t *b); /// Opens a scene buffer
void open_events_buffer(buffers_t *b); /// Opens the events buffer

void open_create_scene_menu(buffers_t *b); /// Opens the create scene menu.

#endif
