/// Handles projects and scenes

#ifndef PROJECT_H
#define PROJECT_H

#include <stddef.h>
#include <stdint.h>

#include "term.h"
#include "buffer.h"

#define PROJECT_DIR "./projects"

/// Represents a vector
typedef struct vector {
	double x, y, z;
} vector_t;

typedef struct scene scene_t;

/// An entity in the scene. Entities are movable objects that are animted by events.
typedef struct entity {
	uint64_t id; /// ID of the entity
	scene_t *parent; /// Scene this entity is in
	vector_t pos; /// The *initial* position of the entity.
	
	color_t color; /// Color of this entity
	char *text; /// Text that makes up this entity.
} entity_t;

/// Represents a single character in the scene
typedef struct scene_char {
	color_t fg, bg; /// Foreground and background colors
	char ch; /// The character
} scene_char_t;

typedef enum event_type {
	EVENT_MOVE_ENTITY
} event_type_t;

// event types
typedef struct e_move_entity {
	uint64_t id; /// ID of the entity
	vector_t off; /// Final offset of the entity
} e_move_entity_t;

/// Represents an event. An event is something that animates things.
typedef struct event {
	event_type_t type; /// Type of the event
	double time, duration; /// Time and duration of the event. 
	union {
		e_move_entity_t move_entity;
	};
} event_t;

/// Represents a scene
typedef struct scene {
	size_t width, length, height; /// Width, height, and length of the scene
	scene_char_t *chars; /// Array of characters. Indexed by [z*width*height+y*width+x]
	
	size_t entities_len; /// Length of the entity array
	entity_t *entities; /// Entities in this scene. Entities at the end of the array are rendered last. Order matters.
	
	size_t events_len; /// Length of the events array
	event_t *events; /// Events

} scene_t;

scene_t *scene_create(size_t w, size_t l, size_t h, scene_char_t fill); /// Create a scene with a given size and char to fill it
void scene_free(scene_t *s);
scene_char_t *scene_getch(scene_t *scene, size_t x, size_t y, size_t z); /// Gets a char in a scene

/// Represents a project
typedef struct project {
	char *name; /// Project name
	char *scene_name; /// Name of the currently loaded scene
	scene_t *scene; /// Currently loaded scene
} project_t;

project_t *project_create(char *name); /// Creates a new project
void project_free(project_t *p); /// Free a project

extern project_t *curr_project;

void project_init(void); /// Initializes the project module

void open_projects_menu(buffers_t *b); /// Opens the project menu
void open_new_project_menu(buffers_t *b); /// Opens the new project menu
void open_project_menu(buffers_t *b); /// Opens a project menu

/// Render a scene to a buffer
/// `this` is the buffer to render to
/// `x`, `y`, and `z` control the X, Y, and Z offset.
/// `move_camera` determines whether move camera events should be processed
/// `time` determines the time (in seconds) to render the scene at
void scene_render(buffer_t *this, scene_t *scene, bool move_camera, double time, size_t x, size_t y, size_t z);

#endif
