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
  const char **keys;
} DBKeys;

typedef enum DBModelType
{
  DBModelType_Object,
  DBModelType_Array,
  DBModelType_String,
  DBModelType_Number,
  DBModelType_Boolean,
  DBModelType_Null,
  DBModelAttr_MaxLength,
  DBModelAttr_MinLength
} DBModelType;

typedef struct DBModel
{
  const char *key;
  DBModelType type;
  int intvalue;
  struct DBModel **attributes;
} DBModel;

typedef struct DBModelArrayProps
{
  DBModel *type;
  // If min_length is -1, it means there is no min_length to the length.
  int min_length;
  // If max_length is -1, it means there is no max_length to the length.
  int max_length;
} DBModelArrayProps;

DBKeys *get_model_keys(DBModel *model);
DBKeys *get_cjson_keys(cJSON *json);
void free_keys(DBKeys *keys);
DBKeys *get_database_keys();
DBModelArrayProps *parse_array_model(DBModel *model);

bool exists(const char *key);
DBItem *get_item(const char *key);
DBItem *set_item(const char *key, cJSON *json);
DBItem *rename_item(const char *old_key, const char *new_key);
bool delete_item(const char *key);

DBModel *def_model(DBModel *parent, const char *key, DBModelType type);
DBModel *def_model_attr(DBModel *model, DBModelType attribute, int value);

void init_database(const char *filename);
void save_database(const char *filename);

#endif
