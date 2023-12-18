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
	QKEY_CALLBACK_RETURN_NORMAL = 1,
	QKEY_CALLBACK_RETURN_IGNORE = 0,
	QKEY_CALLBACK_RETURN_IDK = -1
};

typedef enum QKeyCallbackReturn(*vQKeyCallback)(int, enum QKeyType, va_list);

int qgetch(enum QKeyType* key_type);
void waitForKey(const int key);
int getNumber(int start, int end, const vQKeyCallback key_callback);
int vgetNumber(int start, int end, const vQKeyCallback key_callback, ...);
void getString(char* buffer, int min_len, int max_len, const vQKeyCallback key_callback);
void vgetString(char* buffer, int min_len, int max_len, const vQKeyCallback key_callback, ...);