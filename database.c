#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./database.h"
#include "./cJSON.h"

#define HASH_MOD 5831
#define HASH_SHIFT_BITS 5
#define HASH_TABLE_SIZE 137

cJSON *json_root = NULL;
DBItem **hash_table = NULL;

unsigned long hash(const char *id);

DBItem *_register_item(cJSON *json);

bool has_key(const char *key);
DBItem *get_item(const char *key);
DBItem *create_item(const char *key, cJSON *json);
DBItem *move_item(const char *oldKey, const char *newKey);
bool delete_item(const char *key);

void init_database(const char *filename);
void save_database(const char *filename);

unsigned long hash(const char *string)
{
  if (string == NULL)
    return 0;

  unsigned long hash_value = HASH_MOD;
  int current_char;
  while ((current_char = *string++))
  {
    hash_value = ((hash_value << HASH_SHIFT_BITS) + hash_value) + current_char;
  }
  return hash_value % HASH_TABLE_SIZE;
}

// Construct DBItem with cJSON, then add the cJSON to the json root.
DBItem *_register_item(cJSON *json)
{
  if (json == NULL)
    return NULL;
  DBItem *item = (DBItem *)malloc(sizeof(DBItem));
  item->key = json->string;
  item->json = json;
  unsigned long index = hash(item->key);
  item->next = hash_table[index];
  hash_table[index] = item;
  return item;
}

bool has_key(const char *key)
{
  return get_item(key) != NULL;
}

DBItem *get_item(const char *key)
{
  if (key == NULL)
    return NULL;

  unsigned long index = hash(key);
  DBItem *item = hash_table[index];

  while (item != NULL)
  {
    if (strcmp(item->key, key) == 0)
      return item;
    item = item->next;
  }

  return NULL;
}

DBItem *create_item(const char *key, cJSON *json)
{
  if (key == NULL || json == NULL || has_key(key))
    return NULL;

  cJSON_AddItemToObject(json_root, key, json);
  return _register_item(json);
}

DBItem *move_item(const char *oldKey, const char *newKey)
{
  if (oldKey == NULL || newKey == NULL || has_key(newKey))
    return NULL;

  cJSON *json = cJSON_Duplicate(cJSON_GetObjectItem(json_root, oldKey), true);
  delete_item(oldKey);
  return create_item(newKey, json);
}

// Return true if success, false if fail.
bool delete_item(const char *key)
{
  if (key == NULL)
    return false;

  unsigned long index = hash(key);
  DBItem *prev = NULL;
  DBItem *curr = hash_table[index];

  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      if (prev == NULL)
        hash_table[index] = curr->next;
      else
        prev->next = curr->next;
      free(curr);
      cJSON_DeleteItemFromObject(json_root, key);
      return true;
    }
    prev = curr;
    curr = curr->next;
  }

  return false;
}

void init_database(const char *filename)
{
  // Clear table if table is not empty
  if (hash_table != NULL)
  {
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
      DBItem *item = hash_table[i];
      while (item != NULL)
      {
        DBItem *temp = item;
        item = item->next;
        free(temp);
      }
    }
    free(hash_table);
  };

  // Create hash table
  hash_table = (DBItem **)calloc(HASH_TABLE_SIZE, sizeof(DBItem *));

  // Read the JSON file
  FILE *file = fopen(filename, "r");
  if (file == NULL)
    return;

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *data = (char *)malloc(length + 1);
  fread(data, 1, length, file);
  fclose(file);
  // Prevent memory leak
  data[length] = '\0';

  if (json_root != NULL)
    cJSON_Delete(json_root);
  json_root = cJSON_Parse(data);
  if (json_root == NULL)
    json_root = cJSON_CreateObject();

  free(data);

  // Load data
  cJSON *cursor = json_root->child;
  while (cursor != NULL)
  {
    _register_item(cursor);
    cursor = cursor->next;
  }
}

void save_database(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (file == NULL)
    return;

  char *data = cJSON_Print(json_root);
  fprintf(file, "%s", data);
  free(data);
  fclose(file);
}
