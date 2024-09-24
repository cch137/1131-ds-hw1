#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./cJSON.h"

typedef struct Person
{
  char *name;
  char *jobTitle;
  int age;
  char *address;
  char **phoneNumbers;
  int phoneCount;
  char **emailAddresses;
  int emailCount;
  bool isMarried;
  bool isEmployed;
  struct Person *next;
} Person;

Person *personsHead = NULL;

// Utils

Person *personConstructor(cJSON *personJSON);
cJSON *personJSONConstructor(Person *person);
void clearPerson(Person *person);
void clearDatabase();
void initDatabase(const char *filename);
void saveDatabase(const char *filename);

// CRUD operations

uint8_t createPerson(Person *newPerson);
Person *findPerson(const char *name);
bool updatePerson(const char *name, Person *updatedPerson);
bool deletePerson(const char *name);
bool exists(const char *name);

// Transfer Person cJSON to Person struct
Person *personConstructor(cJSON *personJSON)
{
  Person *person = (Person *)malloc(sizeof(Person));

  person->name = strdup(cJSON_GetObjectItem(personJSON, "name")->valuestring);
  person->jobTitle = strdup(cJSON_GetObjectItem(personJSON, "jobTitle")->valuestring);
  person->age = cJSON_GetObjectItem(personJSON, "age")->valueint;
  person->address = strdup(cJSON_GetObjectItem(personJSON, "address")->valuestring);
  person->isMarried = cJSON_IsTrue(cJSON_GetObjectItem(personJSON, "isMarried"));
  person->isEmployed = cJSON_IsTrue(cJSON_GetObjectItem(personJSON, "isEmployed"));

  cJSON *phoneArray = cJSON_GetObjectItem(personJSON, "phoneNumbers");
  person->phoneCount = cJSON_GetArraySize(phoneArray);
  person->phoneNumbers = (char **)malloc(person->phoneCount * sizeof(char *));
  for (int j = 0; j < person->phoneCount; ++j)
  {
    person->phoneNumbers[j] = strdup(cJSON_GetArrayItem(phoneArray, j)->valuestring);
  }

  cJSON *emailArray = cJSON_GetObjectItem(personJSON, "emailAddresses");
  person->emailCount = cJSON_GetArraySize(emailArray);
  person->emailAddresses = (char **)malloc(person->emailCount * sizeof(char *));
  for (int j = 0; j < person->emailCount; ++j)
  {
    person->emailAddresses[j] = strdup(cJSON_GetArrayItem(emailArray, j)->valuestring);
  }

  return person;
}

// Transfer Person struct to Person cJSON
cJSON *personJSONConstructor(Person *person)
{
  cJSON *personJSON = cJSON_CreateObject();
  cJSON_AddStringToObject(personJSON, "name", person->name);
  cJSON_AddStringToObject(personJSON, "jobTitle", person->jobTitle);
  cJSON_AddNumberToObject(personJSON, "age", person->age);
  cJSON_AddStringToObject(personJSON, "address", person->address);
  cJSON_AddBoolToObject(personJSON, "isMarried", person->isMarried);
  cJSON_AddBoolToObject(personJSON, "isEmployed", person->isEmployed);

  cJSON *phoneArray = cJSON_CreateArray();
  for (int i = 0; i < person->phoneCount; ++i)
  {
    cJSON_AddItemToArray(phoneArray, cJSON_CreateString(person->phoneNumbers[i]));
  }
  cJSON_AddItemToObject(personJSON, "phoneNumbers", phoneArray);

  cJSON *emailArray = cJSON_CreateArray();
  for (int i = 0; i < person->emailCount; ++i)
  {
    cJSON_AddItemToArray(emailArray, cJSON_CreateString(person->emailAddresses[i]));
  }
  cJSON_AddItemToObject(personJSON, "emailAddresses", emailArray);

  return personJSON;
}

// Clear all Key values ​​of Person. (But the "next" pointer will not be cleared)
void clearPerson(Person *person)
{
  if (person == NULL)
    return;

  if (person->name)
    free(person->name);
  if (person->jobTitle)
    free(person->jobTitle);
  if (person->address)
    free(person->address);
  if (person->phoneNumbers)
  {
    for (int i = 0; i < person->phoneCount; i++)
    {
      if (person->phoneNumbers[i])
      {
        free(person->phoneNumbers[i]);
        person->phoneNumbers[i] = NULL;
      }
    }
    free(person->phoneNumbers);
    person->phoneNumbers = NULL;
  }
  if (person->emailAddresses)
  {
    for (int i = 0; i < person->emailCount; i++)
    {
      if (person->emailAddresses[i])
      {
        free(person->emailAddresses[i]);
        person->emailAddresses[i] = NULL;
      }
    }
    free(person->emailAddresses);
    person->emailAddresses = NULL;
  }
}

// Clear the database and free allocated memory.
void clearDatabase()
{
  if (personsHead == NULL)
    return;
  Person *cursor = personsHead;
  while (cursor != NULL)
  {
    Person *p = cursor;
    cursor = cursor->next;
    clearPerson(p);
    free(p);
  }
  personsHead = NULL;
}

// Initialize database from JSON file
void initDatabase(const char *filename)
{
  // If the database has been initialized, clear the current data.
  clearDatabase();

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

  cJSON *json = cJSON_Parse(data);
  free(data);
  if (json == NULL)
    return;

  cJSON *personsArray = cJSON_GetObjectItem(json, "persons");

  Person *cursor = NULL;
  for (int i = cJSON_GetArraySize(personsArray) - 1; i >= 0; --i)
  {
    Person *person = personConstructor(cJSON_GetArrayItem(personsArray, i));
    person->next = cursor;
    cursor = person;
  }

  personsHead = cursor;

  cJSON_Delete(json);
}

// Save the database to a JSON file
void saveDatabase(const char *filename)
{
  cJSON *root = cJSON_CreateObject();
  cJSON *personsArray = cJSON_CreateArray();
  Person *cursor = personsHead;

  while (cursor != NULL)
  {
    cJSON_AddItemToArray(personsArray, personJSONConstructor(cursor));
    cursor = cursor->next;
  }

  cJSON_AddItemToObject(root, "persons", personsArray);

  char *jsonData = cJSON_Print(root);
  FILE *file = fopen(filename, "w");
  fwrite(jsonData, 1, strlen(jsonData), file);
  fclose(file);

  cJSON_Delete(root);
  free(jsonData);
}

// Add a new person into the list
// Returns 0 if the person was successfully added
// Returns 1 if the person is NULL
// Returns 2 if the person name already exists
uint8_t createPerson(Person *newPerson)
{
  if (newPerson == NULL)
    return 1;

  // Check if the person name already exists
  Person *cursor = personsHead;
  while (cursor != NULL)
  {
    if (strcmp(cursor->name, newPerson->name) == 0)
    {
      return 2;
    }
    cursor = cursor->next;
  }

  newPerson->next = personsHead;
  personsHead = newPerson;
  return 0;
}

// Find a person by name
Person *findPerson(const char *name)
{
  Person *current = personsHead;
  while (current != NULL)
  {
    if (strcmp(current->name, name) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

// Update an existing person
bool updatePerson(const char *name, Person *updatedPerson)
{
  Person *existingPerson = findPerson(name);
  if (existingPerson == NULL || updatedPerson == NULL)
    return false;

  clearPerson(existingPerson);
  existingPerson->name = strdup(updatedPerson->name);
  existingPerson->jobTitle = strdup(updatedPerson->jobTitle);
  existingPerson->age = updatedPerson->age;
  existingPerson->address = strdup(updatedPerson->address);
  existingPerson->isMarried = updatedPerson->isMarried;
  existingPerson->isEmployed = updatedPerson->isEmployed;

  existingPerson->phoneNumbers = (char **)malloc(updatedPerson->phoneCount * sizeof(char *));
  for (int i = 0; i < updatedPerson->phoneCount; ++i)
  {
    existingPerson->phoneNumbers[i] = strdup(updatedPerson->phoneNumbers[i]);
  }
  existingPerson->phoneCount = updatedPerson->phoneCount;

  existingPerson->emailAddresses = (char **)malloc(updatedPerson->emailCount * sizeof(char *));
  for (int i = 0; i < updatedPerson->emailCount; ++i)
  {
    existingPerson->emailAddresses[i] = strdup(updatedPerson->emailAddresses[i]);
  }
  existingPerson->emailCount = updatedPerson->emailCount;

  return true;
}

// Delete a person by name
// Returns true if the person was deleted, false otherwise
bool deletePerson(const char *name)
{
  Person *cursor = personsHead;
  Person *prev = NULL;

  while (cursor != NULL)
  {
    if (strcmp(cursor->name, name) == 0)
    {
      // If the person is head, update the head pointer
      if (prev == NULL)
        personsHead = cursor->next;
      // Else bypass the next node to the "next" of the previous node
      else
        prev->next = cursor->next;

      clearPerson(cursor);
      free(cursor);

      return true;
    }
    prev = cursor;
    cursor = cursor->next;
  }

  return false;
}

// Check is name exists or not
bool exists(const char *name)
{
  Person *cursor = personsHead;
  while (cursor != NULL)
  {
    if (strcmp(cursor->name, name) == 0)
      return true;
    cursor = cursor->next;
  }
  return false;
}