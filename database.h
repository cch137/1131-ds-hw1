#ifndef CCH137_DATABASE_H
#define CCH137_DATABASE_H

#include <stdbool.h>
#include "./cJSON.h"

typedef struct DBItem
{
  const char *key;
  cJSON *json;
  struct DBItem *next;
} DBItem;

bool has_key(const char *key);
DBItem *get_item(const char *key);
DBItem *create_item(const char *key, cJSON *json);
DBItem *move_item(const char *oldKey, const char *newKey);
bool delete_item(const char *key);

void init_database(const char *filename);
void save_database(const char *filename);

#endif
