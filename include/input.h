#pragma once
#include <stdarg.h>

#include "base.h"

#define KEY_ARROW_UP 256
#define KEY_ARROW_DOWN 257
#define KEY_ARROW_RIGHT 258
#define KEY_ARROW_LEFT 259

#define KEY_PAGE_UP 260
#define KEY_PAGE_DOWN 261

#define KEY_QUIT -1
#define KEY_QUIT_ALL -2

#define KEY_IS_ARROWS(key) (key >= KEY_ARROW_UP && key <= KEY_PAGE_DOWN)
#define KEY_IS_ARROW(key) (key >= KEY_ARROW_UP && key <= KEY_ARROW_LEFT)
#define KEY_IS_TERMINATING(key) (key < 0)
#define KEY_IS_NORMAL(key) (key >= 0 && key < KEY_ARROW_UP)

enum QKeyCallbackReturn
{
	QKEY_CALLBACK_RETURN_NORMAL,
	QKEY_CALLBACK_RETURN_IGNORE,
	QKEY_CALLBACK_RETURN_END
};

typedef enum QKeyCallbackReturn(*QKeyCallback)(int, va_list);

#define ESC_CHAR 27
#define DEL_CHAR 127

void tot_sleep(unsigned long ms);
Coord getScreenSize(void);
int getKeyInput(void);
void waitForKey(int key);
int getNumberInput(unsigned start, unsigned end, bool erase, const QKeyCallback key_callback, ...);
bool getStringInput(char* buffer, int min_len, int max_len, const QKeyCallback key_callback, ...);
bool getBooleanInput(const QKeyCallback key_callback);

extern bool IS_TTY;
