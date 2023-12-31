#pragma once
#include <stdarg.h>

enum QKeyType {
	QKEY_TYPE_NORMAL,
	QKEY_TYPE_ARROW
};

enum QArrowKey {
	QARROW_UP = 72,
	QARROW_DOWN = 80,
	QARROW_RIGHT = 77,
	QARROW_LEFT = 75
};

enum QKeyCallbackReturn {
	QKEY_CALLBACK_RETURN_NORMAL,
	QKEY_CALLBACK_RETURN_IGNORE,
	QKEY_CALLBACK_RETURN_END
};

typedef enum QKeyCallbackReturn(*vQKeyCallback)(unsigned, enum QKeyType, va_list);

int qgetch(enum QKeyType* key_type);
void waitForKey(const int key);
int getNumber(unsigned start, unsigned end, bool erase, const vQKeyCallback key_callback);
int vgetNumber(unsigned start, unsigned end, bool erase, const vQKeyCallback key_callback, ...);
void getString(char* buffer, int min_len, int max_len, const vQKeyCallback key_callback);
void vgetString(char* buffer, int min_len, int max_len, const vQKeyCallback key_callback, ...);