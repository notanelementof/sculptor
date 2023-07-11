#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "menu.h"
#include "../term.h"

static bool menu_key(buffer_t *this, int key) { 
	menu_t *menu = (menu_t *)this->data;
	switch(key) {
		case 'w':
		case 'W':
			if(menu->locked)
				break;
			if(menu->selected != 0)
				menu->selected--;
			return true;
		case 's':
		case 'S':
			if(menu->locked)
				break;
			if(menu->selected+1 < menu->len)
				menu->selected++;
			return true;
	}
	menu_item_t *sel = &menu->items[menu->selected];
	if(sel->key != NULL && sel->key(sel, key))
		return true;
	return false;
}

static void menu_render(buffer_t *this) {
	menu_t *menu = (menu_t *)this->data;
	size_t y = (this->parent->term->height-1)/2-menu->len/2;
	size_t x = this->width/2;
	for(size_t i = 0; i < menu->len; i++) {
		menu_item_t *mi = &menu->items[i];
		if(mi->text == NULL)
			continue;
		char *text = mi->text(mi);
		buffer_printf(this, x-strlen(text)/2, y+i, i == menu->selected ? menu->locked ? COLOR_LIGHT_RED : COLOR_LIGHT_GREEN : COLOR_WHITE, COLOR_BLACK, "%s", text);
		free(text);
	}
	if(menu->msg != NULL)
		buffer_printf(this, 0, this->height-1, COLOR_RED, COLOR_BLACK, menu->msg);
}

static void menu_free(buffer_t *this) {
	menu_t *menu = (menu_t *)this->data;
	for(size_t i = 0; i < menu->len; i++) {
		menu_item_t *mi = &menu->items[i];
		if(mi->free != NULL)
			mi->free(mi);
		free(mi->data);
	}
	free(menu->items);
	free(menu->msg);
}

menu_info_t menu_create(buffers_t *b, bool set_active, char *name, uint32_t size, size_t len, ...) {
	menu_t *menu = malloc(sizeof(menu_t));
	menu_item_t *items = malloc(len*sizeof(menu_item_t));

	va_list ap;
	va_start(ap, len);
	for(size_t i = 0; i < len; i++) {
		menu_item_t mi = va_arg(ap, menu_item_t);
		mi.parent = menu;
		items[i] = mi;
	}
	va_end(ap);

	*menu = (menu_t){len, 0, items, false, NULL, b};
	uint64_t id = next_buffer_id();
	buffer_t *buf = buffers_add(b, set_active, id, menu, strdup(name), size, &menu_key, &menu_render, &menu_free);
	menu->buffer = buf;
	return (menu_info_t){id, buf, menu};
}

static char *label_text(menu_item_t *this) {
	return strdup((char *)this->data);
}

menu_item_t menu_label(char *text) {
	return (menu_item_t){ITEM_TYPE_OTHER, strdup(text), NULL, &label_text, NULL, NULL};
}

typedef struct button {
	void *data;
	void (*action)(menu_item_t *, void *);
	char *text;
} button_t;

static char *button_text(menu_item_t *mi) {
	return strdup(((button_t *)(mi->data))->text);
}

static bool button_key(menu_item_t *mi, int key) {
	button_t *b = (button_t *)mi->data;
	if(key != KEY_ENTER)
		return false;
	if(b->action == NULL)
		return false;
	b->action(mi, b->data);
	return true;
}

static void button_free(menu_item_t *mi) {
	free(((button_t *)(mi->data))->text);
}

menu_item_t menu_button(char *text, void (*action)(menu_item_t *, void *), void *data) {
	button_t *b = malloc(sizeof(button_t));
	*b = (button_t){data, action, malloc(strlen(text)+5)};
	sprintf(b->text, "[ %s ]", text);	
	return (menu_item_t){ITEM_TYPE_OTHER, b, NULL, &button_text, &button_key, &button_free};
}

typedef struct textbox {
	char *label, *value, *id;
	size_t len;
} textbox_t;

static char *textbox_text(menu_item_t *this) {
	textbox_t *tb = (textbox_t *)this->data;
	char *str = malloc(4096);
	size_t i;
	// TODO: bounds checking
	for(i = 0; i < strlen(tb->label); i++)
		str[i] = tb->label[i];
	str[i++] = ':';
	str[i++] = ' ';
	str[i++] = '<';
	bool passed = false;
	for(size_t j = 0; j < tb->len-1; j++) {
		if(tb->value[j] == '\0')
			passed = true;
		if(passed)
			str[i++] = '_';
		else
			str[i++] = tb->value[j];
	}
	str[i++] = '>';
	str[i++] = '\0';
	return str;
}

static bool textbox_key(menu_item_t *this, int key) {
	textbox_t *tb = (textbox_t *)this->data;
	if(!this->parent->locked) {
		if(key == KEY_ENTER) {
			this->parent->locked = true;
			return true;
		}
		return false;
	}
	switch(key) {
		case KEY_ENTER:
		case KEY_ESCAPE:
			this->parent->locked = false;
			return true;
		case KEY_BACKSPACE: {
			size_t len = strlen(tb->value);
			if(len == 0)
				return true;
			tb->value[len-1] = '\0';
			return true;
		}
		default:
			if(key >= 0x20 && key <= 0x7E) {
				size_t len = strlen(tb->value);
				if(len+1 >= tb->len)
					return true;
				tb->value[len] = key;
				tb->value[len+1] = '\0';
				return true;
			}
	}
	return false;
}

static void textbox_free(menu_item_t *this) {
	free(((textbox_t *)(this->data))->value);
}

menu_item_t menu_textbox(char *label, char *default_value, char *id, size_t len) {
	textbox_t *tb = malloc(sizeof(textbox_t));
	char *value = malloc(len+1);
	strcpy(value, default_value);
	*tb = (textbox_t){label, value, id, len};
	return (menu_item_t){ITEM_TYPE_TEXTBOX, tb, NULL, &textbox_text, &textbox_key, &textbox_free};
}

typedef struct numbox {
	char *label, *id;
	double value, min, max, step;
} numbox_t;

static char *numbox_text(menu_item_t *this) {
	numbox_t *nb = (numbox_t *)this->data;
	char *text = malloc(4096);
	snprintf(text, 4096, "%s (+/-): %.2f", nb->label, nb->value);
	return text;
}

static bool numbox_key(menu_item_t *this, int key) {
	numbox_t *nb = (numbox_t *)this->data;
	switch(key) {
		case '+':
			nb->value += nb->step;
			if(!isnan(nb->max) && nb->value >= nb->max)
				nb->value = nb->max;
			return true;
		case '-':
			nb->value -= nb->step;
			if(!isnan(nb->min) && nb->value <= nb->min)
				nb->value = nb->min;
			return true;
	}
	return false;
}

menu_item_t menu_numbox(char *label, double default_value, double min, double max,  double step, char *id) {
	numbox_t *nb = malloc(sizeof(numbox_t));
	*nb = (numbox_t){label, id, default_value, min, max, step};
	return (menu_item_t){ITEM_TYPE_NUMBOX, nb, NULL, &numbox_text, &numbox_key, NULL};
}

char *menu_get_textbox_value(menu_t *menu, char *id) {
	for(size_t i = 0; i < menu->len; i++) {
		menu_item_t *mi = &menu->items[i];
		if(mi->type != ITEM_TYPE_TEXTBOX)
			continue;
		textbox_t *tb = (textbox_t *)mi->data;
		if(strcmp(tb->id, id) == 0)
			return tb->value;
	}
	return NULL;
}

double menu_get_numbox_value(menu_t *menu, char *id) {
	for(size_t i = 0; i < menu->len; i++) {
		menu_item_t *mi = &menu->items[i];
		if(mi->type != ITEM_TYPE_NUMBOX)
			continue;
		numbox_t *nb = (numbox_t *)mi->data;
		if(strcmp(nb->id, id) == 0)
			return nb->value;
	}
	return NAN;
}

void menu_add_item(menu_t *menu, menu_item_t item) {
	menu->items = realloc(menu->items, sizeof(menu_item_t)*(++menu->len));
	menu->items[menu->len-1] = item;
}

void menu_msg(menu_t *menu, char *msg) {
	if(menu->msg != NULL)
		free(menu->msg);
	menu->msg = strdup(msg);
}

void menu_close(menu_t *menu) {
	buffers_remove(menu->buffers, menu->buffer->id);
}
