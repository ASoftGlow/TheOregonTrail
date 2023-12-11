#pragma once

// puts without newline
#define puts_n(string) fputs(string,stdout)

// strcat but single char
char* strcat_ch(char* dst, const char src);

// strlen but ignores ANSI escape codes
size_t strlen_iae(const char* str);

void clear_stdout(void);