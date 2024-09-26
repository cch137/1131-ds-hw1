#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./interface.h"
#include "./cJSON.h"
#include "./database.h"

char *inputLine();
char inputChar();

void print_person(DBItem *item);
void create_person();
void read_person();
void update_person();
void delete_person();
void main_menu();

#define INPUT_LINE_CHUNK_SIZE 8

char *inputLine()
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

char inputChar()
{
  char *buffer = inputLine();
  char firstChar = buffer[0];
  free(buffer);
  return firstChar;
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

void create_person()
{
  char *tempString = NULL;
  cJSON *json = cJSON_CreateObject();

  printf("Enter the following details:\n");

  printf("Name: ");
  while (1)
  {
    tempString = inputLine();
    cJSON_AddStringToObject(json, "name", tempString);
    if (!has_key(tempString))
      break;
    printf("Name already exists. Please choose a different name.\nName: ");
    free(tempString);
    continue;
  }
  // keep name for later use
  char *name = tempString;

  printf("Job Title: ");
  tempString = inputLine();
  cJSON_AddStringToObject(json, "jobTitle", tempString);
  free(tempString);

  printf("Age: ");
  tempString = inputLine();
  cJSON_AddNumberToObject(json, "age", atoi(tempString));
  free(tempString);

  printf("Address: ");
  tempString = inputLine();
  cJSON_AddStringToObject(json, "address", tempString);
  free(tempString);

  printf("Number of phone numbers: ");
  tempString = inputLine();
  int tempArrayCOunt = atoi(tempString);
  free(tempString);
  cJSON *phoneNumbers = cJSON_CreateArray();
  for (int i = 0; i < tempArrayCOunt; i++)
  {
    printf("Phone Number %d: ", i + 1);
    tempString = inputLine();
    cJSON_AddItemToArray(phoneNumbers, cJSON_CreateString(tempString));
    free(tempString);
  }
  cJSON_AddItemToObject(json, "phoneNumbers", phoneNumbers);

  printf("Number of email addresses: ");
  tempString = inputLine();
  tempArrayCOunt = atoi(tempString);
  free(tempString);
  cJSON *emailAddresses = cJSON_CreateArray();
  for (int i = 0; i < tempArrayCOunt; i++)
  {
    printf("Email Address %d: ", i + 1);
    tempString = inputLine();
    cJSON_AddItemToArray(emailAddresses, cJSON_CreateString(tempString));
    free(tempString);
  }
  cJSON_AddItemToObject(json, "emailAddresses", emailAddresses);

  printf("Married (y/n): ");
  char tempChar = inputChar();
  cJSON_AddBoolToObject(json, "isMarried", tempChar == 'y' || tempChar == 'Y' ? true : false);

  printf("Employed (y/n): ");
  tempChar = inputChar();
  cJSON_AddBoolToObject(json, "isEmployed", tempChar == 'y' || tempChar == 'Y' ? true : false);

  create_item(name, json);
  free(name);
}

void read_person()
{
  printf("Enter the name of the person: ");
  char *name = inputLine();
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
  char *tempString = inputLine();
  DBItem *item = get_item(tempString);
  free(tempString);

  if (item == NULL)
  {
    printf("Person not found.\n");
    return;
  }

  cJSON *json = item->json;

  printf("Choose field to update:\n");
  printf("1. Name\n");
  printf("2. Job Title\n");
  printf("3. Age\n");
  printf("4. Address\n");
  printf("5. Phone Numbers\n");
  printf("6. Email Addresses\n");
  printf("7. Married\n");
  printf("8. Employed\n");
  printf("Your choice: ");

  tempString = inputLine();
  int choice = atoi(tempString);
  free(tempString);

  switch (choice)
  {
  case 1:
  {
    printf("Enter new name: ");
    tempString = inputLine();
    cJSON_ReplaceItemInObject(json, "name", cJSON_CreateString(tempString));
    move_item(item->key, tempString);
    free(tempString);
    break;
  }
  case 2:
  {
    printf("Enter new job title: ");
    tempString = inputLine();
    cJSON_ReplaceItemInObject(json, "jobTitle", cJSON_CreateString(tempString));
    free(tempString);
    break;
  }
  case 3:
  {
    printf("Enter new age: ");
    tempString = inputLine();
    cJSON_ReplaceItemInObject(json, "age", cJSON_CreateNumber(atoi(tempString)));
    free(tempString);
    break;
  }
  case 4:
  {
    printf("Enter new address: ");
    tempString = inputLine();
    cJSON_ReplaceItemInObject(json, "address", cJSON_CreateString(tempString));
    free(tempString);
    break;
  }
  case 5:
  {
    printf("Choose action to phone numbers:\n");
    printf("1. Add\n");
    printf("2. Remove\n");
    printf("Your choice: ");

    cJSON *phoneNumbers = cJSON_GetObjectItem(json, "phoneNumbers");
    int tempArraySize = cJSON_GetArraySize(phoneNumbers);

    switch (inputChar())
    {
    case '1':
    {
      printf("Enter new phone number: ");
      tempString = inputLine();
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
        printf("%d. %s\n", i + 1, cJSON_GetArrayItem(phoneNumbers, i)->valuestring);
      printf("Enter the index of the phone number to remove (start with 1): ");

      tempString = inputLine();
      int index = atoi(tempString);
      free(tempString);

      if (index >= 0 && index < tempArraySize)
        cJSON_DetachItemFromArray(phoneNumbers, index);

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
    printf("1. Add\n");
    printf("2. Remove\n");
    printf("Your choice: ");

    cJSON *emailAddresses = cJSON_GetObjectItem(json, "emailAddresses");
    int tempArraySize = cJSON_GetArraySize(emailAddresses);

    switch (inputChar())
    {
    case '1':
    {
      printf("Enter new email address: ");
      tempString = inputLine();
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
        printf("%d. %s\n", i + 1, cJSON_GetArrayItem(emailAddresses, i)->valuestring);
      printf("Enter the index of the email address to remove (start with 1): ");

      tempString = inputLine();
      int index = atoi(tempString);
      free(tempString);

      if (index >= 0 && index < tempArraySize)
        cJSON_DetachItemFromArray(emailAddresses, index);

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
    char tempChar = inputChar();
    cJSON_ReplaceItemInObject(json, "isMarried", cJSON_CreateBool(tempChar == 'y' || tempChar == 'Y' ? true : false));
    break;
  }
  case 8:
  {
    printf("Employed (y/n): ");
    char tempChar = inputChar();
    cJSON_ReplaceItemInObject(json, "isEmployed", cJSON_CreateBool(tempChar == 'y' || tempChar == 'Y' ? true : false));
    break;
  }
  }
}

void delete_person()
{
  printf("Enter the name of the person to delete: ");
  char *name = inputLine();
  bool result = delete_item(name);
  free(name);

  if (result)
    printf("Person deleted successfully.\n");
  else
    printf("Person not found.\n");
}

void main_menu()
{
  while (1)
  {
    printf("\n################ Main Menu ################\n");
    printf("Welcome to CCH's address book!!!\n");
    printf("Choose an option:\n");
    printf("C. Create a new person\n");
    printf("R. Read a person\n");
    printf("U. Update a person\n");
    printf("D. Delete a person\n");
    printf("X. Exit\n");
    printf("Your choice: ");

    switch (inputChar())
    {
    case 'C':
    case 'c':
      create_person();
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
