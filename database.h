#ifndef CCH137_DATABASE_H
#define CCH137_DATABASE_H

#include <stdbool.h>
#include "./cJSON.h"

typedef struct DBItem
{
  char *key;
  cJSON *json;
  struct DBItem *next;
} DBItem;

typedef struct DBKeys
{
  int length;
  char **keys;
} DBKeys;

DBKeys *get_keys();
void free_keys(DBKeys *keys);

bool exists(const char *key);
DBItem *get_item(const char *key);
DBItem *set_item(const char *key, cJSON *json);
DBItem *rename_item(const char *oldKey, const char *newKey);
bool delete_item(const char *key);

typedef enum DBModelType
{
  DBModelType_Object,
  DBModelType_Array,
  DBModelType_String,
  DBModelType_Number,
  DBModelType_Boolean,
  DBModelType_Null
} DBModelType;

typedef struct DBModel
{
  const char *key;
  DBModelType type;
  int length;
  struct DBModel **attributes;
} DBModel;

DBModel *model(const char *key, DBModelType type);
DBModel *define_model(DBModel *model, DBModel *attribute);

void init_database(const char *filename);
void save_database(const char *filename);

#endif
