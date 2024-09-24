#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./cJSON.h"

// Define the Person struct (now with a linked list pointer)
struct Person
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
  struct Person *next; // Pointer to the next person in the list
};

// Global variable to store the head of the linked list
struct Person *persons = NULL;

// Function prototypes
struct Person *createPersonFromJSON(cJSON *json);
void freePerson(struct Person *p);
void parseJSONFile(const char *filename);
void writeJSONFile(const char *filename);
void findPerson(const char *name);
void insertPerson(struct Person *newPerson);
void updatePerson(const char *name, struct Person *updatedPerson);
void deletePerson(const char *name);

// Create a new Person from JSON data
struct Person *createPersonFromJSON(cJSON *json)
{
  if (json == NULL)
    return NULL;

  struct Person *p = (struct Person *)malloc(sizeof(struct Person));
  if (p == NULL)
  {
    printf("Memory allocation for Person struct failed\n");
    exit(1);
  }
  memset(p, 0, sizeof(struct Person)); // Initialize everything to zero

  cJSON *item;

  item = cJSON_GetObjectItem(json, "name");
  p->name = item ? strdup(item->valuestring) : NULL;

  item = cJSON_GetObjectItem(json, "jobTitle");
  p->jobTitle = item ? strdup(item->valuestring) : NULL;

  item = cJSON_GetObjectItem(json, "address");
  p->address = item ? strdup(item->valuestring) : NULL;

  item = cJSON_GetObjectItem(json, "age");
  p->age = item ? item->valueint : 0;

  item = cJSON_GetObjectItem(json, "isMarried");
  p->isMarried = item ? item->valueint : false;

  item = cJSON_GetObjectItem(json, "isEmployed");
  p->isEmployed = item ? item->valueint : false;

  cJSON *phoneArray = cJSON_GetObjectItem(json, "phoneNumbers");
  if (phoneArray)
  {
    p->phoneCount = cJSON_GetArraySize(phoneArray);
    p->phoneNumbers = (char **)malloc(p->phoneCount * sizeof(char *));
    for (int i = 0; i < p->phoneCount; ++i)
    {
      cJSON *phoneItem = cJSON_GetArrayItem(phoneArray, i);
      p->phoneNumbers[i] = phoneItem ? strdup(phoneItem->valuestring) : NULL;
    }
  }

  cJSON *emailArray = cJSON_GetObjectItem(json, "emailAddresses");
  if (emailArray)
  {
    p->emailCount = cJSON_GetArraySize(emailArray);
    p->emailAddresses = (char **)malloc(p->emailCount * sizeof(char *));
    for (int i = 0; i < p->emailCount; ++i)
    {
      cJSON *emailItem = cJSON_GetArrayItem(emailArray, i);
      p->emailAddresses[i] = emailItem ? strdup(emailItem->valuestring) : NULL;
    }
  }

  p->next = NULL; // Ensure the next pointer is initialized to NULL
  return p;
}

// Free memory allocated for a Person
void freePerson(struct Person *p)
{
  if (p == NULL)
    return;

  if (p->name)
    free(p->name);
  if (p->jobTitle)
    free(p->jobTitle);
  if (p->address)
    free(p->address);

  if (p->phoneNumbers)
  {
    for (int i = 0; i < p->phoneCount; i++)
    {
      if (p->phoneNumbers[i])
        free(p->phoneNumbers[i]);
    }
    free(p->phoneNumbers);
  }

  if (p->emailAddresses)
  {
    for (int i = 0; i < p->emailCount; i++)
    {
      if (p->emailAddresses[i])
        free(p->emailAddresses[i]);
    }
    free(p->emailAddresses);
  }

  free(p);
}

// Parse JSON file and initialize a linked list of Person structs
void parseJSONFile(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error opening file %s\n", filename);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *data = (char *)malloc(length + 1);
  fread(data, 1, length, file);
  fclose(file);
  data[length] = '\0';

  cJSON *json = cJSON_Parse(data);
  free(data);
  if (json == NULL)
  {
    printf("Error parsing JSON\n");
    exit(1);
  }

  cJSON *personsArray = cJSON_GetObjectItem(json, "persons");
  if (personsArray == NULL)
  {
    printf("No 'persons' array found in JSON\n");
    cJSON_Delete(json);
    exit(1);
  }

  int personCount = cJSON_GetArraySize(personsArray);
  for (int i = 0; i < personCount; ++i)
  {
    cJSON *personJSON = cJSON_GetArrayItem(personsArray, i);
    struct Person *newPerson = createPersonFromJSON(personJSON);
    insertPerson(newPerson);
  }

  cJSON_Delete(json);
}

// Write the updated Person linked list to a JSON file
void writeJSONFile(const char *filename)
{
  cJSON *root = cJSON_CreateObject();
  cJSON *personsArray = cJSON_CreateArray();

  struct Person *current = persons;
  while (current)
  {
    cJSON *personJSON = cJSON_CreateObject();
    cJSON_AddStringToObject(personJSON, "name", current->name ? current->name : "");
    cJSON_AddStringToObject(personJSON, "jobTitle", current->jobTitle ? current->jobTitle : "");
    cJSON_AddNumberToObject(personJSON, "age", current->age);
    cJSON_AddStringToObject(personJSON, "address", current->address ? current->address : "");

    cJSON *phoneArray = cJSON_CreateArray();
    for (int i = 0; i < current->phoneCount; ++i)
    {
      cJSON_AddItemToArray(phoneArray, cJSON_CreateString(current->phoneNumbers[i]));
    }
    cJSON_AddItemToObject(personJSON, "phoneNumbers", phoneArray);

    cJSON *emailArray = cJSON_CreateArray();
    for (int i = 0; i < current->emailCount; ++i)
    {
      cJSON_AddItemToArray(emailArray, cJSON_CreateString(current->emailAddresses[i]));
    }
    cJSON_AddItemToObject(personJSON, "emailAddresses", emailArray);

    cJSON_AddBoolToObject(personJSON, "isMarried", current->isMarried);
    cJSON_AddBoolToObject(personJSON, "isEmployed", current->isEmployed);

    cJSON_AddItemToArray(personsArray, personJSON);
    current = current->next;
  }

  cJSON_AddItemToObject(root, "persons", personsArray);
  char *jsonData = cJSON_Print(root);
  FILE *file = fopen(filename, "w");
  fwrite(jsonData, 1, strlen(jsonData), file);
  fclose(file);
  cJSON_Delete(root);
  free(jsonData);
}

// Find a person by name
void findPerson(const char *name)
{
  struct Person *current = persons;
  while (current)
  {
    if (strcmp(current->name, name) == 0)
    {
      printf("Found Person:\nName: %s\nJob Title: %s\nAge: %d\nAddress: %s\n", current->name, current->jobTitle, current->age, current->address);
      return;
    }
    current = current->next;
  }
  printf("Person with name %s not found\n", name);
}

// Insert a new person (linked list insertion at the beginning)
void insertPerson(struct Person *newPerson)
{
  newPerson->next = persons;
  persons = newPerson;
}

// Update an existing person
void updatePerson(const char *name, struct Person *updatedPerson)
{
  struct Person *current = persons;
  while (current)
  {
    if (strcmp(current->name, name) == 0)
    {
      freePerson(current);
      *current = *updatedPerson;
      return;
    }
    current = current->next;
  }
  printf("Person with name %s not found\n", name);
}

// Delete a person by name
void deletePerson(const char *name)
{
  struct Person *current = persons;
  struct Person *prev = NULL;

  while (current)
  {
    if (strcmp(current->name, name) == 0)
    {
      if (prev)
        prev->next = current->next;
      else
        persons = current->next;

      freePerson(current);
      return;
    }
    prev = current;
    current = current->next;
  }
  printf("Person with name %s not found\n", name);
}

// Main function for testing
int main()
{
  // Initialize by reading a JSON file
  parseJSONFile("database.json");

  // Test finding a person
  findPerson("John Doe");

  // Insert a new person
  struct Person *newPerson = (struct Person *)malloc(sizeof(struct Person));
  newPerson->name = strdup("Jane Smith");
  newPerson->jobTitle = strdup("Software Developer");
  newPerson->age = 28;
  newPerson->address = strdup("456 Another St");
  newPerson->phoneNumbers = NULL;
  newPerson->phoneCount = 0;
  newPerson->emailAddresses = NULL;
  newPerson->emailCount = 0;
  newPerson->isMarried = false;
  newPerson->isEmployed = true;
  newPerson->next = NULL;
  insertPerson(newPerson);

  deletePerson("Alice");

  // Write the updated list to the JSON file
  writeJSONFile("updated_database.json");

  // Cleanup and exit
  struct Person *current = persons;
  while (current)
  {
    struct Person *next = current->next;
    freePerson(current);
    current = next;
  }

  return 0;
}
