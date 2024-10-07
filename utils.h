#ifndef CCH137_UTILS_H
#define CCH137_UTILS_H

void memory_error_handler(const char *filename, int line, const char *funcname);

char *input_string();
int input_int();
double input_double();
char input_char();

char *int_to_string(int value);

void print_tabs(int depth, bool end_with_dash);

#define INPUT_STRING_CHUNK_SIZE 8

#endif
