#pragma once

#define ANSI_COLOR_RED         "\033[31m"
#define ANSI_COLOR_GREEN       "\033[32m"
#define ANSI_COLOR_YELLOW      "\033[33m"
#define ANSI_COLOR_BLUE        "\033[34m"
#define ANSI_COLOR_MAGENTA     "\033[35m"
#define ANSI_COLOR_CYAN        "\033[36m"
#define ANSI_COLOR_BROWN       "\033[38;5;94m"
#define ANSI_SELECTED	       "\033[30;47m"
#define ANSI_COLOR_RESET       "\033[0m"
#define ANSI_BG_RESET          "\033[40m"
#define ANSI_CURSOR_SHOW	   "\033[?25h"
#define ANSI_CURSOR_HIDE	   "\033[?25l"
#define ANSI_CURSOR_SAVE       "\033[s"
#define ANSI_CURSOR_RESTORE    "\033[u"
#define ANSI_CURSOR_POS(x, y)  "\033[" x ";" y "H"
#define ANSI_CURSOR_STYLE_UNDERLINE "\033[3 q"
#define ANSI_CURSOR_STYLE_DEFAULT "\033[0q"
#define ANSI_BG_BROWN    "\033[48;5;94m"
#define ANSI_BOLD "\033[4m"
#define ANSI_WINDOW_TITLE(title) "\33]0;"title"\33\\"
#define ANSI_WINDOW_SIZE(width, height) "\33[8;"height";"width"t"
#define ANSI_SB_ALT "\33[?1049h"
#define ANSI_SB_MAIN "\33[?1049l"