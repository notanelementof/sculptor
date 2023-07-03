#ifndef TERM_H
#define TERM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


/// Represents a color
typedef struct color {
	uint8_t r, g, b;
	bool transparent;
} color_t;

color_t color(uint8_t r, uint8_t g, uint8_t b);

#define TRANSPARENT (color_t){0, 0, 0, true}
#define COLOR_BLACK (color_t){0, 0, 0, false}
#define COLOR_BLUE (color_t){0, 0, 128, false}
#define COLOR_GREEN (color_t){0, 128, 0, false}
#define COLOR_CYAN (color_t){0, 128, 128, false}
#define COLOR_RED (color_t){128, 0, 0, false}
#define COLOR_MAGENTA (color_t){128, 0, 128, false}
#define COLOR_YELLOW (color_t){128, 128, 0, false}
#define COLOR_LIGHT_GRAY (color_t){192, 192, 192, false}
#define COLOR_DARK_GRAY (color_t){255, 255, 255, false}
#define COLOR_LIGHT_BLUE (color_t){0, 0, 255, false}
#define COLOR_LIGHT_GREEN (color_t){0, 255, 0, false}
#define COLOR_LIGHT_CYAN (color_t){0, 255, 255, false}
#define COLOR_LIGHT_RED (color_t){255, 0, 0, false}
#define COLOR_LIGHT_MAGENTA (color_t){255, 0, 255, false}
#define COLOR_LIGHT_YELLOW (color_t){255, 255, 0, false}
#define COLOR_WHITE (color_t){255, 255, 255, false}

/// A terminal character
typedef struct tchar {
	char ch; // character
	// color
	// f* = foreground
	// b* = background
	uint8_t fr, fg, fb, br, bg, bb;
} tchar_t;

/// Represents a terminal
typedef struct terminal {
	size_t width, height;

	tchar_t *curr; /// Current buffer
	tchar_t *prev; /// Previous buffer
} terminal_t;

/// Size of a terminal
typedef struct termsize {
	size_t width, height;
} termsize_t;

// input functions
void init_termios(void); /// Sets up the termios for getch() and kbhit()
void reset_termios(void); /// Resets termios (automatically registered atexit by init_termios())
int getch(void); /// Get a char
bool kbhit(void); /// Returns true if the keyboard is hit

termsize_t get_term_size(void); /// Gets the current terminal size

terminal_t *term_new(void); /// Creates a new terminal with the current terminal size
void term_free(terminal_t *term); /// Frees a terminal
void term_resize(terminal_t *term, termsize_t size); /// Resizes a terminal
void term_flush(terminal_t *term); /// Flushes a terminal
void term_putch(terminal_t *term, size_t x, size_t y, color_t fg, color_t bg, char ch); /// Puts a character at a given position and color
void term_clear(terminal_t *term, color_t col); /// Clears a terminal with a given color

#endif
