#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./interface.h"
#include "./cJSON.h"
#include "./database.h"

char *input_string();
int input_int();
char input_char();
char *int_to_string(int value);
cJSON *input_cjson(DBModel *model, int depth);

#define INPUT_LINE_CHUNK_SIZE 8

char *input_string()
{
  size_t bufferSize = INPUT_LINE_CHUNK_SIZE;
  char *buffer = malloc(bufferSize);
  if (!buffer)
    return NULL;

  size_t index = 0;
  int c;

  while (1)
  {
    c = fgetc(stdin);

    if (c == EOF || c == '\n')
    {
      buffer[index] = '\0';
      break;
    }

    if (index >= bufferSize - 1)
    {
      bufferSize += INPUT_LINE_CHUNK_SIZE;
      buffer = realloc(buffer, bufferSize);
      if (!buffer)
        return NULL;
    }

    buffer[index++] = c;
  }

  if (index == 0 && c == EOF)
  {
    free(buffer);
    return NULL;
  }

  buffer = realloc(buffer, index + 1);
  return buffer;
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
  char *string = (char *)calloc((length), sizeof(char));
  sprintf(string, "%d", value);
  string[length - 1] = '\0';
  return string;
}

cJSON *input_cjson(DBModel *model, int depth)
{
  switch (model->type)
  {
  case DBModelType_Object:
  {
    cJSON *object = cJSON_CreateObject();
    int _depth = depth;
    int length = model->length;
    DBModel *attribute = NULL;

    while (_depth--)
      printf(_depth ? "  " : "- ");
    printf("<Object> %s:\n", model->key);

    for (int i = 0; i < length; i++)
    {
      attribute = model->attributes[i];

      if (attribute == NULL)
        continue;

      cJSON_AddItemToObject(object, attribute->key, input_cjson(attribute, depth + 1));
    }

    return object;
  }

  case DBModelType_Array:
  {
    int _depth = depth;
    char *buffer = NULL;
    cJSON *array = cJSON_CreateArray();

    while (_depth--)
      printf(_depth ? "  " : "- ");
    printf("<Array> %s\n", model->key);
    _depth = depth;
    while (_depth--)
      printf("  ");
    printf("length: ");

    int length = input_int();

    for (int i = 0; i < length; i++)
    {
      buffer = int_to_string(i + 1);
      model->attributes[0]->key = buffer;

      cJSON_AddItemToArray(array, input_cjson(model->attributes[0], depth + 1));

      free(buffer);
    }
    return array;
  }

  case DBModelType_String:
    while (depth--)
      printf(depth ? "  " : "- ");
    printf("<String> %s: ", model->key);
    return cJSON_CreateString(input_string());

  case DBModelType_Number:
    while (depth--)
      printf(depth ? "  " : "- ");
    printf("<Number> %s: ", model->key);
    return cJSON_CreateNumber(input_double());

  case DBModelType_Boolean:
  {
    while (depth--)
      printf(depth ? "  " : "- ");
    printf("<Boolean> %s (y/n): ", model->key);
    char choice = input_char();
    return cJSON_CreateBool(choice == 'y' || choice == 'Y');
  }

  case DBModelType_Null:
    return cJSON_CreateNull();

  default:
    return NULL;
  }
}

void print_person(DBItem *item)
{
  if (item == NULL)
    return;

  cJSON *json = item->json;

  printf("----------------------------------------------------------------\n");

  printf("%-16s: %s\n", "Name", cJSON_GetObjectItem(json, "name")->valuestring);
  printf("%-16s: %s\n", "Job Title", cJSON_GetObjectItem(json, "jobTitle")->valuestring);
  printf("%-16s: %d\n", "Age", cJSON_GetObjectItem(json, "age")->valueint);
  printf("%-16s: %s\n", "Address", cJSON_GetObjectItem(json, "address")->valuestring);

  cJSON *phoneNumbers = cJSON_GetObjectItem(json, "phoneNumbers");
  int tempArraySize = cJSON_GetArraySize(phoneNumbers);
  printf("%-16s: ", tempArraySize > 1 ? "Phone Numbers" : "Phone Number");
  for (int i = 0; i < tempArraySize; i++)
    printf("%s%s", cJSON_GetArrayItem(phoneNumbers, i)->valuestring, i == tempArraySize - 1 ? "\n" : ", ");

  cJSON *emailAddresses = cJSON_GetObjectItem(json, "emailAddresses");
  tempArraySize = cJSON_GetArraySize(emailAddresses);
  printf("%-16s: ", tempArraySize > 1 ? "Email Addresses" : "Email Address");
  for (int i = 0; i < tempArraySize; i++)
    printf("%s%s", cJSON_GetArrayItem(emailAddresses, i)->valuestring, i == tempArraySize - 1 ? "\n" : ", ");

  printf("%-16s: %s\n", "Married", (cJSON_GetObjectItem(json, "isMarried")->valueint) ? "YES" : "NO");
  printf("%-16s: %s\n", "Employed", (cJSON_GetObjectItem(json, "isEmployed")->valueint) ? "YES" : "NO");

  printf("----------------------------------------------------------------\n");
}

void create_person(DBModel *person_model)
{
  cJSON *person_json = input_cjson(person_model, 0);
  char *name = NULL;

  if (exists(cJSON_GetObjectItem(person_json, "name")->valuestring))
  {
    while (true)
    {
      char *buffer = NULL;
      printf("Person with this name already exists. Enter a different name: ");
      buffer = input_string();
      cJSON_ReplaceItemInObject(person_json, "name", cJSON_CreateString(buffer));
      if (exists(buffer))
      {
        free(buffer);
        continue;
      }
      printf("Person has been successfully created.\n");
      set_item(buffer, person_json);
      free(buffer);
      break;
    }
  }
  else
  {
    set_item(name, person_json);
  }
}

void read_person()
{
  printf("Enter the name of the person: ");
  char *name = input_string();
  DBItem *item = get_item(name);
  free(name);

  if (item == NULL)
    printf("Person not found.\n");
  else
    print_person(item);
}

void update_person()
{
  printf("Enter the name of the person to update: ");
  char *tempString = input_string();
  DBItem *item = get_item(tempString);
  free(tempString);

  if (item == NULL)
  {
    printf("Person not found.\n");
    return;
  }

  cJSON *json = item->json;

  printf("Choose field to update:\n");
  printf("1 - Name\n");
  printf("2 - Job Title\n");
  printf("3 - Age\n");
  printf("4 - Address\n");
  printf("5 - Phone Numbers\n");
  printf("6 - Email Addresses\n");
  printf("7 - Married\n");
  printf("8 - Employed\n");
  printf("Your choice: ");

  int choice = input_int();

  switch (choice)
  {
  case 1:
  {
    printf("Enter new name: ");
    tempString = input_string();
    if (exists(tempString))
    {
      printf("Person with this name already exists. Operation canceled.\n");
      free(tempString);
      break;
    }
    cJSON_ReplaceItemInObject(json, "name", cJSON_CreateString(tempString));
    rename_item(item->key, tempString);
    free(tempString);
    break;
  }
  case 2:
  {
    printf("Enter new job title: ");
    tempString = input_string();
    cJSON_ReplaceItemInObject(json, "jobTitle", cJSON_CreateString(tempString));
    free(tempString);
    break;
  }
  case 3:
  {
    printf("Enter new age: ");
    cJSON_ReplaceItemInObject(json, "age", cJSON_CreateNumber(input_int()));
    break;
  }
  case 4:
  {
    printf("Enter new address: ");
    tempString = input_string();
    cJSON_ReplaceItemInObject(json, "address", cJSON_CreateString(tempString));
    free(tempString);
    break;
  }
  case 5:
  {
    printf("Choose action to phone numbers:\n");
    printf("1 - Add\n");
    printf("2 - Remove\n");
    printf("Your choice: ");

    cJSON *phoneNumbers = cJSON_GetObjectItem(json, "phoneNumbers");
    int tempArraySize = cJSON_GetArraySize(phoneNumbers);

    switch (input_char())
    {
    case '1':
    {
      printf("Enter new phone number: ");
      tempString = input_string();
      cJSON_AddItemToArray(phoneNumbers, cJSON_CreateString(tempString));
      free(tempString);
      break;
    }
    case '2':
    {
      if (tempArraySize == 0)
      {
        printf("No phone numbers to remove.\n");
        break;
      }

      printf("Phone numbers:\n");
      for (int i = 0; i < tempArraySize; i++)
        printf("%d) %s\n", i + 1, cJSON_GetArrayItem(phoneNumbers, i)->valuestring);
      printf("Enter the index of the phone number to remove (start with 1): ");

      int index = input_int();
      if (index >= 0 && index < tempArraySize)
        cJSON_DeleteItemFromArray(phoneNumbers, index);

      break;
    }
    default:
      break;
    }
    break;
  }
  case 6:
  {
    printf("Choose action to email addresses:\n");
    printf("1 - Add\n");
    printf("2 - Remove\n");
    printf("Your choice: ");

    cJSON *emailAddresses = cJSON_GetObjectItem(json, "emailAddresses");
    int tempArraySize = cJSON_GetArraySize(emailAddresses);

    switch (input_char())
    {
    case '1':
    {
      printf("Enter new email address: ");
      tempString = input_string();
      cJSON_AddItemToArray(emailAddresses, cJSON_CreateString(tempString));
      free(tempString);
      break;
    }
    case '2':
    {
      if (tempArraySize == 0)
      {
        printf("No email addresses to remove.\n");
        break;
      }

      printf("Email addresses:\n");
      for (int i = 0; i < tempArraySize; i++)
        printf("%d) %s\n", i + 1, cJSON_GetArrayItem(emailAddresses, i)->valuestring);
      printf("Enter the index of the email address to remove (start with 1): ");

      int index = input_int();
      if (index >= 0 && index < tempArraySize)
        cJSON_DeleteItemFromArray(emailAddresses, index);

      break;
    }
    default:
      break;
    }
    break;
  }
  case 7:
  {
    printf("Married (y/n): ");
    char tempChar = input_char();
    cJSON_ReplaceItemInObject(json, "isMarried", cJSON_CreateBool(tempChar == 'y' || tempChar == 'Y' ? true : false));
    break;
  }
  case 8:
  {
    printf("Employed (y/n): ");
    char tempChar = input_char();
    cJSON_ReplaceItemInObject(json, "isEmployed", cJSON_CreateBool(tempChar == 'y' || tempChar == 'Y' ? true : false));
    break;
  }
  }
}

void delete_person()
{
  printf("Enter the name of the person to delete: ");
  char *name = input_string();
  bool result = delete_item(name);
  free(name);

  if (result)
    printf("Person deleted successfully.\n");
  else
    printf("Person not found.\n");
}

void main_menu()
{
  DBModel *person_model = model("Person", DBModelType_Object);
  define_model(person_model, model("name", DBModelType_String));
  define_model(person_model, model("jobTitle", DBModelType_String));
  define_model(person_model, model("age", DBModelType_Number));
  define_model(person_model, model("address", DBModelType_String));
  define_model(define_model(person_model, model("phoneNumbers", DBModelType_Array)), model(NULL, DBModelType_String));
  define_model(define_model(person_model, model("emailAddresses", DBModelType_Array)), model(NULL, DBModelType_String));
  define_model(person_model, model("isMarried", DBModelType_Boolean));
  define_model(person_model, model("isEmployed", DBModelType_Boolean));

  while (1)
  {
    printf("\n################ Main Menu ################\n");
    printf("Welcome to CCH's address book!!!\n");
    printf("Choose an option:\n");
    printf("C - Create a new person\n");
    printf("R - Read a person\n");
    printf("U - Update a person\n");
    printf("D - Delete a person\n");
    printf("K - List keys\n");
    printf("S - Save database\n");
    printf("X - Exit\n");
    printf("Your choice: ");

    switch (input_char())
    {
    case 'C':
    case 'c':
      create_person(person_model);
      break;

    case 'R':
    case 'r':
      read_person();
      break;

    case 'U':
    case 'u':
      update_person();
      break;

    case 'D':
    case 'd':
      delete_person();
      break;

    case 'S':
    case 's':
      save_database("database.json");
      printf("Database saved successfully.\n");
      break;

    case 'K':
    case 'k':
    {
      DBKeys *keys = get_keys();
      for (int i = 0; i < keys->length; i++)
        printf("%d) %s\n", i + 1, keys->keys[i]);
      free_keys(keys);
      break;
    }

    case 'X':
    case 'x':
      printf("Exiting... Good bye!\n");
      return;

    default:
      printf("Invalid choice.\n");
      break;
    }
  }
}
