#pragma once

#define ANSI_COLOR_RED         "\33[31m"
#define ANSI_COLOR_GREEN       "\33[32m"
#define ANSI_COLOR_YELLOW      "\33[33m"
#define ANSI_COLOR_BLUE        "\33[34m"
#define ANSI_COLOR_MAGENTA     "\33[35m"
#define ANSI_COLOR_CYAN        "\33[36m"
#define ANSI_COLOR_BROWN       "\33[38;5;94m"
#define ANSI_COLOR_GRAY        "\33[38;5;8m"
#define ANSI_SELECTED	       "\33[30;47m"
#define ANSI_COLOR_RESET       "\33[0m"
#define ANSI_BG_RESET          "\33[40m"
#define ANSI_CURSOR_SHOW	   "\33[?25h"
#define ANSI_CURSOR_HIDE	   "\33[?25l"
#define ANSI_CURSOR_SAVE       "\33[s"
#define ANSI_CURSOR_RESTORE    "\33[u"
#define ANSI_CURSOR_POS(x, y)  "\33[" y ";" x "H"
#define ANSI_CURSOR_STYLE_DEFAULT   "\33[0 q"
#define ANSI_CURSOR_STYLE_BLOCK     "\33[1 q"
#define ANSI_CURSOR_STYLE_UNDERLINE "\33[3 q"
#define ANSI_CURSOR_ZERO "\33[1;1H"
#define ANSI_BG_BROWN    "\33[48;5;94m"
#define ANSI_DIM "\33[2m"
#define ANSI_NO_DIM "\33[22m"
#define ANSI_UNDERLINE "\33[4m"
#define ANSI_NO_UNDERLINE "\33[22m"
#define ANSI_WINDOW_TITLE(title) "\33]0;"title"\33\\"
#define ANSI_WINDOW_SIZE(width, height) "\33[8;"height";"width"t"
#define ANSI_SB_ALT "\33[?1049h"
#define ANSI_SB_MAIN "\33[?1049l"
#define ANSI_NO_WRAP "\33[?7l"
#define ANSI_WRAP "\33[?7h"