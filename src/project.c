#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "buffers/menu.h"
#include "buffers/scene.h"
#include "main.h"
#include "util.h"

#include "project.h"

void project_init(void) {
	struct stat st;
	if(stat(PROJECT_DIR, &st) == -1)
		mkdir(PROJECT_DIR, 0700);
}

menu_info_t menu_create(buffers_t *b, bool set_active, char *name, uint32_t size, size_t len, ...); /// Creates a menu in the given buffer group and returns a struct with some info about the menu

static void back_cb(menu_item_t *this, void *data) {
	open_main_menu((buffers_t *)data);
}

static void new_project_cb(menu_item_t *this, void *data) {
	open_new_project_menu((buffers_t *)data);
}

void open_projects_menu(buffers_t *b) {
	buffers_clear(b);
	menu_t *menu = menu_create(b, true, "Projects", 5, 3,
		menu_button("New Project", &new_project_cb, b),
		menu_button("Back", &back_cb, b),
		menu_label("< - >")
	).menu;
}

void create_project_cb(menu_item_t *this, void *data) {
	char *name = menu_get_textbox_value(this->parent, "name");
	char *err = validate_name(name);
	if(err != NULL) {
		menu_msg(this->parent, err);
		return;
	}
	project_t *project = project_create(name);
	if(project != NULL) {
		curr_project = project;
		open_project_menu((buffers_t *)data);
	}
}

void cancel_project_cb(menu_item_t *this, void *data) {
	open_projects_menu((buffers_t *)data);
}

void open_new_project_menu(buffers_t *b) {
	buffers_clear(b);
	menu_create(b, true, "Create Project", 5, 3,
		menu_textbox("Project Name", "", "name", 64),
		menu_button("Create", &create_project_cb, b),
		menu_button("Cancel", &cancel_project_cb, b)
	);
}

project_t *curr_project;

project_t *project_create(char *name) {
	project_t *p = malloc(sizeof(project_t));
	*p = (project_t){strdup(name), NULL, NULL};
	return p;
}

void project_free(project_t *p) {
	free(p->name);
	free(p->scene_name);
	if(p->scene != NULL)
		scene_free(p->scene);
	free(p);
}

scene_t *scene_create(size_t w, size_t l, size_t h, scene_char_t fill) {
	scene_t *scene = malloc(sizeof(scene_t));
	scene_char_t *chars = malloc(sizeof(scene_char_t)*w*l*h);
	for(size_t i = 0; i < w*l*h; i++)
		chars[i] = fill;
	*scene = (scene_t){w, l, h, chars, 0, NULL, 0, NULL};
	return scene;
}

void scene_free(scene_t *s) {
	free(s->chars);
	free(s->entities);
	free(s->events);
	free(s);
}

scene_char_t *scene_getch(scene_t *scene, size_t x, size_t y, size_t z) {
	return &scene->chars[z*scene->width*scene->height+y*scene->width+x];
}

void open_project_menu(buffers_t *b) {
	buffers_clear(b);
	open_scene_buffer(b); 
}


void scene_render(buffer_t *this, scene_t *scene, bool move_camera, double time, size_t x, size_t y, size_t z) {
	for(size_t i = 0; i < this->width; i++) {
		for(size_t j = 0; j < this->height; j++) {
			size_t px = x+i-this->width/2, py = y+j-this->height/2;
			if(px > scene->width || py > scene->length || z > scene->height)
				continue;
			scene_char_t *c = scene_getch(scene, px, py, z);
			buffer_putch(this, i, j, c->fg, c->bg, c->ch);
		}
	}
}
