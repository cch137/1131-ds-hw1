#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./utils.h"

void memory_error_handler(const char *filename, int line, const char *funcname)
{
  printf("Error: Memory allocation failed in '%s' function\n", funcname);
  printf("    at %s:%d\n", filename, line);
  exit(1);
}

char *input_string()
{
  size_t buffer_size = INPUT_STRING_CHUNK_SIZE;
  size_t index = 0;
  char *buffer = (char *)malloc(buffer_size * sizeof(char));

  // return NULL if memory allocation fails
  if (!buffer)
    memory_error_handler(__FILE__, __LINE__, __func__);

  int c;
  // read characters until EOF or newline
  while ((c = fgetc(stdin)) != EOF && c != '\n')
  {
    // check if the buffer needs to be expanded
    if (index >= buffer_size - 1)
    {
      buffer_size += INPUT_STRING_CHUNK_SIZE;
      buffer = (char *)realloc(buffer, buffer_size * sizeof(char));
      if (!buffer)
        memory_error_handler(__FILE__, __LINE__, __func__);
    }
    // store the character in the buffer
    buffer[index++] = (char)c;
  }

  // if EOF is encountered and no characters were read, free and return NULL
  if (index == 0 && c == EOF)
  {
    free(buffer);
    return NULL;
  }

  buffer[index] = '\0'; // Null-terminate the string

  // reallocate memory to match the exact string length
  buffer = (char *)realloc(buffer, (index + 1) * sizeof(char));
  if (!buffer)
    memory_error_handler(__FILE__, __LINE__, __func__);

  return buffer; // return the final string
}

int input_int()
{
  char *buffer = input_string();
  int value = atoi(buffer);
  free(buffer);
  return value;
}

double input_double()
{
  char *buffer = input_string();
  double value = atof(buffer);
  free(buffer);
  return value;
}

char input_char()
{
  char *buffer = input_string();
  char firstChar = buffer[0];
  free(buffer);
  return firstChar;
}

char *int_to_string(int value)
{
  int digit_counter = value;
  int length = value < 0 ? 3 : 2;
  while (digit_counter /= 10)
    length++;
  char *string = (char *)calloc(length, sizeof(char));
  if (!string)
    memory_error_handler(__FILE__, __LINE__, __func__);
  sprintf(string, "%d", value);
  string[length - 1] = '\0';
  return string;
}

void print_tabs(int tab_depth, bool end_with_dash)
{
  if (end_with_dash)
    while (tab_depth--)
      printf(tab_depth ? "  " : "- ");
  else
    while (tab_depth--)
      printf("  ");
}
