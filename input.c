#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <curses.h>
#endif

#include "input.h"

int qgetch(enum QKeyType* key_type)
{
	int key = _getch();
	if (key == 3 || key == 4) exit(-1);
	if (key == 0 || key == 224) { // if the first value is esc
		*key_type = QKEYTYPE_ARROW;
		return _getch();
	}

	*key_type = QKEYTYPE_NORMAL;
	return key;
}

void waitForKey(const int key)
{
	enum QKeyType type;
	while (1)
	{
		if (qgetch(&type) == key && type == QKEYTYPE_NORMAL) break;
	}
}