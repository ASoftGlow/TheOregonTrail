#pragma once

int qgetch(enum QKeyType* key_type);
void waitForKey(const int key);

enum QKeyType {
	QKEYTYPE_NORMAL,
	QKEYTYPE_ARROW
};

enum QArrowKey {
	QARROW_UP = 72,
	QARROW_DOWN = 80,
	QARROW_RIGHT = 77,
	QARROW_LEFT = 75
};