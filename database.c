#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./cJSON.h"
#include "./database.h"

#define HASH_MOD 5831
#define HASH_SHIFT_BITS 5
#define HASH_TABLE_SIZE 137

cJSON *json_root = NULL;
DBItem **hash_table = NULL;

unsigned long static hash(const char *string);
DBItem static *create_item_with_json(const char *key, cJSON *json);
DBItem static *add_item_to_hash_table(const char *key, DBItem *item);
DBItem static *remove_item_from_hash_table(const char *key);
DBItem static *set_item_key(DBItem *item, const char *key);

unsigned long static hash(const char *string)
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

DBItem static *create_item_with_json(const char *key, cJSON *json)
{
  if (json == NULL)
    return NULL;

  DBItem *item = (DBItem *)malloc(sizeof(DBItem));

  if (item == NULL)
  {
    printf("Error: Failed to allocate memory for item\n");
    exit(1);
  }

  item->key = NULL;
  item->json = json;
  item->next = NULL;
  set_item_key(item, key);

  return item;
}

DBItem static *add_item_to_hash_table(const char *key, DBItem *item)
{
  if (item == NULL)
    return NULL;

  unsigned long index = hash(key);
  item->next = hash_table[index];
  hash_table[index] = item;

  return item;
}

DBItem static *remove_item_from_hash_table(const char *key)
{
  if (key == NULL)
    return NULL;

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

      return curr;
    }
    prev = curr;
    curr = curr->next;
  }

  return NULL;
}

DBItem static *set_item_key(DBItem *item, const char *key)
{
  if (item == NULL || key == NULL)
    return NULL;

  size_t size = (strlen(key) + 1) * sizeof(char);
  item->key = (char *)realloc(item->key, size);

  if (item->key == NULL)
  {
    printf("Error: Failed to allocate memory for item->key\n");
    exit(1);
  }

  memset(item->key, 0, size);
  item->key = strcpy(item->key, key);

  return item;
}

DBKeys *get_keys()
{
  DBKeys *keys = (DBKeys *)malloc(sizeof(DBKeys));

  if (keys == NULL)
  {
    printf("Error: Failed to allocate memory for keys\n");
    exit(1);
  }

  keys->length = 0;
  keys->keys = NULL;
  int count = 0;
  DBItem *cursor = NULL;

  for (int i = 0; i < HASH_TABLE_SIZE; i++)
  {
    cursor = hash_table[i];
    while (cursor != NULL)
    {
      count++;
      if (keys->length < count)
      {
        keys->length += 8;
        keys->keys = (char **)realloc(keys->keys, keys->length * sizeof(char *));
        if (keys->keys == NULL)
        {
          printf("Error: Failed to allocate memory for keys->keys\n");
          exit(1);
        }
      }
      keys->keys[count - 1] = cursor->key;
      cursor = cursor->next;
    }
  }

  if (keys->length != count)
  {
    keys->length = count;
    keys->keys = (char **)realloc(keys->keys, count * sizeof(char *));
    if (keys->keys == NULL)
    {
      printf("Error: Failed to allocate memory for keys->keys\n");
      exit(1);
    }
  }

  return keys;
}

void free_keys(DBKeys *keys)
{
  if (keys == NULL)
    return;

  free(keys->keys);
  free(keys);
}

bool exists(const char *key)
{
  return (key != NULL && get_item(key) != NULL);
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

DBItem *set_item(const char *key, cJSON *json)
{
  if (key == NULL || json == NULL)
    return NULL;

  DBItem *oldItem = get_item(key);
  if (oldItem != NULL)
  {
    if (oldItem->json == json)
      return oldItem;
    delete_item(key);
  }

  DBItem *item = create_item_with_json(key, json);
  add_item_to_hash_table(key, item);
  cJSON_AddItemToObject(json_root, key, json);

  return item;
}

DBItem *rename_item(const char *oldKey, const char *newKey)
{
  if (oldKey == NULL || newKey == NULL || exists(newKey))
    return NULL;

  // remove item with old key
  DBItem *item = remove_item_from_hash_table(oldKey);
  cJSON_DetachItemFromObject(json_root, oldKey);

  // add item with new key
  add_item_to_hash_table(newKey, item);
  cJSON_AddItemToObject(json_root, newKey, item->json);

  // rename item
  set_item_key(item, newKey);

  return item;
}

// Return true if success, false if fail.
bool delete_item(const char *key)
{
  DBItem *item = remove_item_from_hash_table(key);

  if (item == NULL)
    return false;

  cJSON_Delete(item->json);
  free(item);

  return true;
}

DBModel *model(const char *key, DBModelType type)
{
  DBModel *model = (DBModel *)malloc(sizeof(DBModel));

  if (model == NULL)
  {
    printf("Error: Failed to create model\n");
    exit(1);
  }

  model->key = key;
  model->type = type;
  model->length = 0;
  model->attributes = NULL;

  return model;
}

DBModel *define_model(DBModel *model, DBModel *attribute)
{
  model->attributes = (DBModel **)realloc(model->attributes, (model->length + 1) * sizeof(DBModel *));

  if (model->attributes == NULL)
  {
    printf("Error: Failed to allocate memory for model->attributes\n");
    exit(1);
  }

  model->attributes[model->length] = attribute;
  model->length++;

  return attribute;
}

void init_database(const char *filename)
{
  // read the JSON file
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error: Failed to open file %s\n", filename);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *db_json_string = (char *)calloc((length + 1), sizeof(char));
  if (db_json_string == NULL)
  {
    printf("Error: Failed to allocate memory for data\n");
    exit(1);
  }
  fread(db_json_string, 1, length, file);
  fclose(file);
  // prevent memory leak
  db_json_string[length] = '\0';

  // clear table if table is not NULL
  if (hash_table != NULL)
  {
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
      DBItem *item = hash_table[i];
      while (item != NULL)
      {
        free(item);
        item = item->next;
      }
    }
    free(hash_table);
    hash_table = NULL;
  };

  // clear json root if json root is not NULL
  if (json_root != NULL)
  {
    cJSON_Delete(json_root);
    json_root = NULL;
  }

  // create hash table
  hash_table = (DBItem **)calloc(HASH_TABLE_SIZE, sizeof(DBItem *));

  if (hash_table == NULL)
  {
    printf("Error: Failed to allocate memory for hash table\n");
    exit(1);
  }

  // create json root
  json_root = cJSON_Parse(db_json_string);
  if (json_root == NULL)
    json_root = cJSON_CreateObject();

  free(db_json_string);

  // load items
  cJSON *json_cursor = json_root->child;
  DBItem *item = NULL;
  while (json_cursor != NULL)
  {
    item = create_item_with_json(json_cursor->string, json_cursor);
    add_item_to_hash_table(json_cursor->string, item);
    json_cursor = json_cursor->next;
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
