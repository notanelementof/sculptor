#ifndef BUFFER_MENU_H
#define BUFFER_MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "../buffer.h"

typedef struct menu menu_t;

typedef struct menu_item menu_item_t;

typedef enum menu_item_type {
	ITEM_TYPE_OTHER, /// for when the type of a menu item is irrelevant
	ITEM_TYPE_TEXTBOX, /// For textbox items 
	ITEM_TYPE_NUMBOX /// For numboxes
} menu_item_type_t;

/// Represents a menu item
typedef struct menu_item {
	menu_item_type_t type;
	void *data; /// Any data
	menu_t *parent; /// Menu that this item is in
	char *(*text)(menu_item_t *this); /// Returns the text that this item should display. The char* returned is freed after use.
	bool (*key)(menu_item_t *this, int key); /// Activates when a key is pressed on this menu item. Returns whether or not the key was consumed
	void (*free)(menu_item_t *this); /// Should free all data associated with this menu item. Do not free the data pointer, is it freed seperately.
} menu_item_t;

/// Represents a menu
typedef struct menu {
	size_t len; /// Length of the menu
	size_t selected; /// Index of the selected item
	menu_item_t *items; /// Menu items
	bool locked; /// Whenever the menu is 'locked' on a specific item. The use is basically just making enterable text boxes
} menu_t;

/// Info about a menu
typedef struct menu_info {
	uint64_t buffer_id; /// ID that the created buffer had
	buffer_t *buffer; /// Pointer to the created buffer
	menu_t *menu; /// Pointer to the menu
} menu_info_t;

menu_info_t menu_create(buffers_t *b, bool set_active, char *name, uint32_t size, size_t len, ...); /// Creates a menu in the given buffer group and returns a struct with some info about the menu
menu_item_t menu_label(char *text); /// Creates a menu label. 
menu_item_t menu_button(char *text, void (*action)(menu_item_t *, void *), void *data); /// Creates a button. When pressed, the action() function is called with the other parameter (data) passed in.
menu_item_t menu_textbox(char *label, char *default_value, char *id, size_t len); /// An editable textbox. strlen(default_value) must be <= len
menu_item_t menu_numbox(char *label, double default_value, double min, double max, double step, char *id); /// A box that can store an editable number. `min`, `max`, or both can be set to NaN to not limit the range of values.

char *menu_get_textbox_value(menu_t *menu, char *id); /// Gets the value of a textbox in a menu. Returns NULL if the textbox can't be found
double menu_get_numbox_value(menu_t *menu, char *id); /// Gets the value of a numbox in a menu. Returns NaN if the numbox can't be found.

#endif
