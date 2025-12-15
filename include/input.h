#pragma once
#include <stdarg.h>

#include "base.h"

enum
{
  KEY_UNKNOWN = 256,
  KEY_ARROW_UP,
  KEY_ARROW_DOWN,
  KEY_ARROW_RIGHT,
  KEY_ARROW_LEFT,

  KEY_PAGE_UP,
  KEY_PAGE_DOWN,
};

#define KEY_QUIT                -1
#define KEY_QUIT_ALL            -2

#define KEY_IS_ARROWS(key)      (key >= KEY_ARROW_UP && key <= KEY_PAGE_DOWN)
#define KEY_IS_ARROW(key)       (key >= KEY_ARROW_UP && key <= KEY_ARROW_LEFT)
#define KEY_IS_TERMINATING(key) (key < 0)
#define KEY_IS_NORMAL(key)      (key >= 0 && key <= 255)

enum QKeyCallbackReturn
{
  QKEY_CALLBACK_RETURN_NORMAL,
  QKEY_CALLBACK_RETURN_IGNORE,
  QKEY_CALLBACK_RETURN_END
};

typedef enum QKeyCallbackReturn (*QKeyCallback)(int, va_list);

#define ESC_CHAR 27
#define DEL_CHAR 127

void tot_sleep(unsigned long ms);
Coord getScreenSize(void);
void setScreenSize(int width, int height);
void disableResizing(void);
void enableResizing(void);
int getKeyInput(void);
void waitForKey(int key);
int getNumberInput(unsigned start, unsigned end, bool erase, const QKeyCallback key_callback, ...);
// returns zero if success
bool getStringInput(char* buffer, int min_len, int max_len, const QKeyCallback key_callback, ...);
bool
getWrappedStringInput(char* buffer, byte width, Coord offset, int min_len, int max_len, const QKeyCallback key_callback, ...);
bool getBooleanInput(const QKeyCallback key_callback);

extern bool IS_TTY;
// Make sure to set back to false consume action and prevent double-escape combo from occurring early.
extern bool escape_combo;
