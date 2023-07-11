#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../project.h"
#include "../main.h"
#include "../term.h"
#include "../util.h"
#include "menu.h"

#include "scene.h"

typedef enum scene_mode {
	SM_VIEW, SM_INSERT
} scene_mode_t;

// struct for scene buffer data
typedef struct sceneb {
	scene_mode_t mode;
	size_t x, y, z; // x y and z positions 
	// view mode
	char *input; // buffer for keybindings (like the one in vim)
	// insert mode
	color_t fg, bg;
} sceneb_t;

static void clear_input(sceneb_t *sb) {
	free(sb->input);
	sb->input = NULL;
}

static bool sceneb_key(buffer_t *this, int key) {
	sceneb_t *sb = (sceneb_t *)this->data;
	switch(sb->mode) {
		case SM_VIEW: {
			if(key == KEY_ESCAPE) {
				clear_input(sb);
				return true;
			}
			if(!((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z')))
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
			if(strcmp(sb->input, "q") == 0) {
				clear_input(sb);
				project_free(curr_project);
				open_main_menu(this->parent);
				return true;
			}
			if(strcmp(sb->input, "Sn") == 0) {
				clear_input(sb);
				open_create_scene_menu(this->parent);
				return true;
			}
			#define MOVE(DEC, INC, COORD, SIZE) \
			if(strcmp(sb->input, DEC) == 0) { \
				clear_input(sb); \
				if(sb->COORD == 0) \
					sb->COORD = curr_project->scene->SIZE-1; \
				else \
					sb->COORD -= 1; \
				return true; \
			} \
			if(strcmp(sb->input, INC) == 0) { \
				clear_input(sb); \
				if(sb->COORD+1 == curr_project->scene->SIZE) \
					sb->COORD = 0; \
				else \
					sb->COORD += 1; \
				return true; \
			}
			MOVE("w", "s", y, length);
			MOVE("a", "d", x, width);
			MOVE(",", ".", z, height);
			#undef MOVE
			return true;
		}
	}
	return false;
}

static void sceneb_render(buffer_t *this) {
	sceneb_t *sb = (sceneb_t *)this->data;
	if(curr_project->scene == NULL) {
		// no scene loaded
		char *text = "No scene loaded. Type `Sn` to create a new scene, or `Sl` to load a scene.";
		buffer_printf(this, this->width/2-strlen(text)/2, this->height/2, COLOR_LIGHT_GRAY, COLOR_BLACK, text);
	} else {
		scene_render(this, curr_project->scene, false, 0, sb->x, sb->y, sb->z);
		// render cursor
		if(millis()%1000 < 500) {
			buffer_putch(this, this->width/2, this->height/2, COLOR_WHITE, COLOR_BLACK, '_');
		}
	}
	if(sb->input != NULL)
		buffer_printf(this, this->width-strlen(sb->input), this->height-1, COLOR_WHITE, COLOR_BLACK, "%s", sb->input);
}

void sceneb_free(buffer_t *this) {
	sceneb_t *sb = (sceneb_t *)this->data;
	free(sb->input);
}

void open_scene_buffer(buffers_t *b) {
	sceneb_t *sb = malloc(sizeof(sceneb_t));
	*sb = (sceneb_t){SM_VIEW, 0, 0, 0, NULL, COLOR_WHITE, COLOR_BLACK};
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
