#ifndef CCH137_DATABASE_H
#define CCH137_DATABASE_H

#include <stdbool.h>
#include <pthread.h>
#include "./cJSON.h"

#define DATABASE_FILENAME "database.json"

extern pthread_mutex_t *db_mutex;

// items

typedef struct DBItem
{
  char *key;
  cJSON *json;
  struct DBItem *next;
} DBItem;

bool exists(const char *key);
DBItem *get_item(const char *key);
DBItem *set_item(const char *key, cJSON *json);
DBItem *rename_item(const char *old_key, const char *new_key);
bool delete_item(const char *key);

// models

#define DBModel_ArrayTypeSymbol NULL

typedef enum DBModelType
{
  DBModelType_Object,
  DBModelType_Array,
  DBModelType_String,
  DBModelType_Number,
  DBModelType_Boolean,
  DBModelType_Null,
  DBModelAttr_ArrayTypeGetter,
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

DBModel *def_model(DBModel *parent, const char *key, DBModelType type);
DBModel *def_model_attr(DBModel *model, DBModelType attribute, int value);
DBModel *get_model_attr(DBModel *model, DBModelType type);

// keys

typedef struct DBKeys
{
  int length;
  const char **keys;
} DBKeys;

DBKeys *get_model_keys(DBModel *model);
DBKeys *get_cjson_keys(cJSON *json);
DBKeys *get_database_keys();
void free_keys(DBKeys *keys);

// database

void load_database(const char *filename);
void save_database(const char *filename);

#endif