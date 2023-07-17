#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../project.h"
#include "../main.h"
#include "../term.h"
#include "../util.h"
#include "menu.h"
#include "colorpicker.h"

#include "scene.h"

typedef enum scene_mode {
	SM_VIEW, SM_INSERT
} scene_mode_t;

// struct for scene buffer data
typedef struct sceneb {
	scene_mode_t mode;
	size_t x, y, z; // x y and z positions 
	color_t msg_color; // color of the message
	char *msg; // a message that can be displayed
	// view mode
	char *input; // buffer for keybindings (like the one in vim)
	// insert mode
	color_t fg, bg;
	char *insert_buf; // buffer for text
} sceneb_t;

static void sceneb_clear_input(sceneb_t *sb) {
	free(sb->input);
	sb->input = NULL;
}

static void sceneb_msg(sceneb_t *sb, color_t col, char *msg) {
	free(sb->msg);
	sb->msg = strdup(msg);
	sb->msg_color = col;
}

static void sceneb_set_mode(sceneb_t *sb, scene_mode_t mode) {
	switch(mode) {
		case SM_INSERT:
			free(sb->insert_buf);
			sb->insert_buf = NULL;
			break;
	}
	sb->mode = mode;
	switch(mode) {
		case SM_VIEW:
			sceneb_msg(sb, COLOR_WHITE, "");
			break;
		case SM_INSERT:
			sceneb_msg(sb, COLOR_WHITE, "-- INSERT --");
			sb->insert_buf = malloc(1);
			sb->insert_buf[0] = '\0';
			break;
	}
}

static void set_color_cb(void *data, color_t col) {
	*(color_t *)data = col;
}

static bool sceneb_key(buffer_t *this, int key) {
	sceneb_t *sb = (sceneb_t *)this->data;
	switch(sb->mode) {
		case SM_VIEW: {
			if(key == KEY_ESCAPE) {
				sceneb_clear_input(sb);
				return true;
			}
			if(!is_printable(key))
				return false;
			// add key to sb->input
			if(sb->input == NULL) {
				sb->input = malloc(1);
				sb->input[0] = '\0';
			}
			size_t len = strlen(sb->input);
			sb->input = realloc(sb->input, len+2);
			sb->input[len] = key;
			sb->input[len+1] = '\0';
			// test sb->input for combinations
			// quit
			if(strcmp(sb->input, "q") == 0) {
				sceneb_clear_input(sb);
				project_free(curr_project);
				open_main_menu(this->parent);
				return true;
			}
			// scene actions
			if(strcmp(sb->input, "cn") == 0) {
				sceneb_clear_input(sb);
				open_create_scene_menu(this->parent);
				return true;
			}
			if(strcmp(sb->input, "cf") == 0) {
				sceneb_clear_input(sb);
				open_colorpicker(this->parent, true, 3, &set_color_cb, sb->fg, &sb->fg);
				return true;
			}
			if(strcmp(sb->input, "cb") == 0) {
				sceneb_clear_input(sb);
				open_colorpicker(this->parent, true, 3, &set_color_cb, sb->bg, &sb->bg);
				return true;
			}
			// movement actions
			#define MOVE(DEC, INC, COORD, SIZE, N) \
			if(strcmp(sb->input, DEC) == 0) { \
				sceneb_clear_input(sb); \
				if(sb->COORD-N <= curr_project->scene->SIZE) \
					sb->COORD -= N; \
				else \
					sb->COORD = 0; \
				return true; \
			} \
			if(strcmp(sb->input, INC) == 0) { \
				sceneb_clear_input(sb); \
				if(sb->COORD+N <= curr_project->scene->SIZE) \
					sb->COORD += N; \
				else \
					sb->COORD = curr_project->scene->SIZE-1; \
				return true; \
			}
			MOVE("w", "s", y, length, 1);
			MOVE("a", "d", x, width, 1);
			MOVE("W", "S", y, length, 5);
			MOVE("A", "D", x, width, 5);
			MOVE(",", ".", z, height, 1);
			#undef MOVE
			// mode changes
			if(strcmp(sb->input, "i") == 0) {
				sceneb_clear_input(sb);
				sceneb_set_mode(sb, SM_INSERT);
				return true;
			}
			return true;
		}
		case SM_INSERT: {
			if(key == KEY_ESCAPE) {
				sceneb_set_mode(sb, SM_VIEW);
				return true;
			}
			size_t len = strlen(sb->insert_buf);
			if(key == KEY_BACKSPACE) {
				if(len != 0) {
					sb->insert_buf[len-1] = '\0';
					sb->insert_buf = realloc(sb->insert_buf, len);
				}
				return true;
			}
			if(key == KEY_ENTER) {
				for(size_t i = 0; i < len; i++)
					scene_putch(curr_project->scene, sb->x+i, sb->y, sb->z, sb->fg, sb->bg, sb->insert_buf[i]);
				sceneb_set_mode(sb, SM_VIEW);
				sb->y++;
				sb->y %= curr_project->scene->height;
				return true;
			}
			if(!is_printable(key))
				return false;
			if(sb->x+1 == curr_project->scene->width)
				return false;
			if(sb->x+len == curr_project->scene->width-1)
				return true;
			sb->insert_buf = realloc(sb->insert_buf, len+2);
			sb->insert_buf[len] = key;
			sb->insert_buf[++len] = '\0';
			return true;
		}
	}
	return false;
}

static void sceneb_render(buffer_t *this) {
	sceneb_t *sb = (sceneb_t *)this->data;
	if(curr_project->scene == NULL) {
		// no scene loaded
		char *text = "No scene loaded. Type `cn` to create a new scene, or `cl` to load a scene.";
		buffer_printf(this, this->width/2-strlen(text)/2, this->height/2, COLOR_LIGHT_GRAY, COLOR_BLACK, text);
	} else {
		scene_render(this, curr_project->scene, false, 0, sb->x+(sb->mode == SM_INSERT ? strlen(sb->insert_buf) : 0), sb->y, sb->z);
		// mode-specific rendering
		switch(sb->mode) {
			case SM_INSERT:
				buffer_printf(this, this->width/2-strlen(sb->insert_buf), this->height/2, sb->fg, sb->bg, "%s", sb->insert_buf);
		}
		// render cursor
		if(millis()%1000 < 500) {
			buffer_putch(this, this->width/2, this->height/2, sb->fg, sb->bg, '_');
		}
	}
	if(sb->input != NULL)
		buffer_printf(this, this->width-strlen(sb->input), this->height-1, COLOR_WHITE, COLOR_BLACK, "%s", sb->input);
	if(sb->msg != NULL)
		buffer_printf(this, 0, this->height-1, sb->msg_color, COLOR_BLACK, "%s", sb->msg);
}

void sceneb_free(buffer_t *this) {
	sceneb_t *sb = (sceneb_t *)this->data;
	free(sb->input);
	free(sb->insert_buf);
	free(sb->msg);
}

void open_scene_buffer(buffers_t *b) {
	sceneb_t *sb = malloc(sizeof(sceneb_t));
	*sb = (sceneb_t){SM_VIEW, 0, 0, 0, COLOR_WHITE, NULL, NULL, COLOR_WHITE, COLOR_BLACK, NULL};
	buffers_add(b, true, next_buffer_id(), sb, strdup("Scene"), 4, &sceneb_key, &sceneb_render, &sceneb_free);
}

menu_info_t menu_create(buffers_t *b, bool set_active, char *name, uint32_t size, size_t len, ...); /// Creates a menu in the given buffer group and returns a struct with some info about the menu

static void create_scene_cb(menu_item_t *this, void *data) {
	menu_t *menu = this->parent;
	char *name = menu_get_textbox_value(menu, "name");
	char *err = validate_name(name);
	if(err != NULL) {
		menu_msg(menu, err);
		return;
	}
	size_t width = (size_t)menu_get_numbox_value(menu, "width");
	size_t length = (size_t)menu_get_numbox_value(menu, "length");
	size_t height = (size_t)menu_get_numbox_value(menu, "height");

	scene_t *scene = scene_create(width, length, height, (scene_char_t){COLOR_WHITE, COLOR_DARK_GRAY, '#'});
	curr_project->scene = scene;
	curr_project->scene_name = strdup(name);

	menu_close(menu);
}

static void create_scene_cancel_cb(menu_item_t *this, void *data) {
	menu_t *menu = this->parent;
	menu_close(menu);
}

void open_create_scene_menu(buffers_t *b) {
	menu_create(b, true, "Create Scene", 4, 6,
		menu_textbox("Name", "", "name", 32),
		menu_numbox("Width", 100, 1, 10000, 10, "width"),
		menu_numbox("Length", 100, 1, 10000, 10, "length"),
		menu_numbox("Height", 20, 1, 10000, 10, "height"),
		menu_button("Create Scene", &create_scene_cb, NULL),
		menu_button("Cancel", &create_scene_cancel_cb, NULL)
	);
}
