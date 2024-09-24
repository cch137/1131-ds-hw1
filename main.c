#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./cJSON.h"

// Define the Person struct
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
  struct Person *next;
};

// Global pointer to the head of the linked list
struct Person *persons = NULL;

// Function prototypes
void freePerson(struct Person *p, bool keepP);
void initDatabase(const char *filename);
void saveDatabase(const char *filename);
struct Person *findPerson(const char *name);
struct Person *insertPerson(struct Person *newPerson);
void updatePerson(const char *name, struct Person *updatedPerson);
bool deletePerson(const char *name);
void printPerson(struct Person *person);

// Free memory allocated for a Person
void freePerson(struct Person *p, bool keepP)
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
      {
        free(p->phoneNumbers[i]);
        p->phoneNumbers[i] = NULL;
      }
    }
    free(p->phoneNumbers);
    p->phoneNumbers = NULL;
  }
  if (p->emailAddresses)
  {
    for (int i = 0; i < p->emailCount; i++)
    {
      if (p->emailAddresses[i])
      {
        free(p->emailAddresses[i]);
        p->emailAddresses[i] = NULL;
      }
    }
    free(p->emailAddresses);
    p->emailAddresses = NULL;
  }

  if (!keepP && p)
    free(p);

  p = NULL;
}

// Initialize database from JSON file
void initDatabase(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
    return;

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
    return;

  cJSON *personsArray = cJSON_GetObjectItem(json, "persons");
  int personCount = cJSON_GetArraySize(personsArray);

  for (int i = 0; i < personCount; ++i)
  {
    cJSON *personJSON = cJSON_GetArrayItem(personsArray, i);
    struct Person *person = (struct Person *)malloc(sizeof(struct Person));

    person->name = strdup(cJSON_GetObjectItem(personJSON, "name")->valuestring);
    person->jobTitle = strdup(cJSON_GetObjectItem(personJSON, "jobTitle")->valuestring);
    person->age = cJSON_GetObjectItem(personJSON, "age")->valueint;
    person->address = strdup(cJSON_GetObjectItem(personJSON, "address")->valuestring);
    person->isMarried = cJSON_IsTrue(cJSON_GetObjectItem(personJSON, "isMarried"));
    person->isEmployed = cJSON_IsTrue(cJSON_GetObjectItem(personJSON, "isEmployed"));

    // Phone numbers
    cJSON *phoneArray = cJSON_GetObjectItem(personJSON, "phoneNumbers");
    person->phoneCount = cJSON_GetArraySize(phoneArray);
    person->phoneNumbers = (char **)malloc(person->phoneCount * sizeof(char *));
    for (int j = 0; j < person->phoneCount; ++j)
    {
      person->phoneNumbers[j] = strdup(cJSON_GetArrayItem(phoneArray, j)->valuestring);
    }

    // Email addresses
    cJSON *emailArray = cJSON_GetObjectItem(personJSON, "emailAddresses");
    person->emailCount = cJSON_GetArraySize(emailArray);
    person->emailAddresses = (char **)malloc(person->emailCount * sizeof(char *));
    for (int j = 0; j < person->emailCount; ++j)
    {
      person->emailAddresses[j] = strdup(cJSON_GetArrayItem(emailArray, j)->valuestring);
    }

    person->next = persons;
    persons = person;
  }
  cJSON_Delete(json);
}

// Save the database to a JSON file
void saveDatabase(const char *filename)
{
  cJSON *root = cJSON_CreateObject();
  cJSON *personsArray = cJSON_CreateArray();
  struct Person *current = persons;

  while (current != NULL)
  {
    cJSON *personJSON = cJSON_CreateObject();
    cJSON_AddStringToObject(personJSON, "name", current->name);
    cJSON_AddStringToObject(personJSON, "jobTitle", current->jobTitle);
    cJSON_AddNumberToObject(personJSON, "age", current->age);
    cJSON_AddStringToObject(personJSON, "address", current->address);
    cJSON_AddBoolToObject(personJSON, "isMarried", current->isMarried);
    cJSON_AddBoolToObject(personJSON, "isEmployed", current->isEmployed);

    // Phone numbers
    cJSON *phoneArray = cJSON_CreateArray();
    for (int i = 0; i < current->phoneCount; ++i)
    {
      cJSON_AddItemToArray(phoneArray, cJSON_CreateString(current->phoneNumbers[i]));
    }
    cJSON_AddItemToObject(personJSON, "phoneNumbers", phoneArray);

    // Email addresses
    cJSON *emailArray = cJSON_CreateArray();
    for (int i = 0; i < current->emailCount; ++i)
    {
      cJSON_AddItemToArray(emailArray, cJSON_CreateString(current->emailAddresses[i]));
    }
    cJSON_AddItemToObject(personJSON, "emailAddresses", emailArray);

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
struct Person *findPerson(const char *name)
{
  struct Person *current = persons;
  while (current != NULL)
  {
    if (strcmp(current->name, name) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

// Insert a new person into the list
struct Person *insertPerson(struct Person *newPerson)
{
  if (newPerson == NULL)
    return NULL;
  newPerson->next = persons;
  persons = newPerson;
  return newPerson;
}

// Update an existing person
void updatePerson(const char *name, struct Person *updatedPerson)
{
  struct Person *existingPerson = findPerson(name);
  if (existingPerson == NULL || updatedPerson == NULL)
    return;

  freePerson(existingPerson, true);
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
}

bool deletePerson(const char *name)
{
  struct Person *current = persons;
  struct Person *prev = NULL;

  while (current != NULL)
  {
    if (strcmp(current->name, name) == 0)
    {
      if (prev == NULL)
        persons = current->next; // Deleting the head

      else
        prev->next = current->next; // Bypass the current node

      freePerson(current, false);

      // Optional: Set pointers to NULL to avoid dangling references
      current = NULL;
      return true;
    }
    prev = current;
    current = current->next;
  }

  return false;
}

// Print a person’s information neatly
void printPerson(struct Person *person)
{
  if (!person)
  {
    printf("Person is NULL.\n");
    return;
  }

  printf("%-15s : %s \n", "Name", person->name ? person->name : "N/A");
  printf("%-15s : %s \n", "Job Title", person->jobTitle ? person->jobTitle : "N/A");
  printf("%-15s : %d \n", "Age", person->age);
  printf("%-15s : %s \n", "Address", person->address ? person->address : "N/A");

  printf("%-15s : ", "Phone Numbers");
  if (person->phoneCount > 0 && person->phoneNumbers)
  {
    for (int i = 0; i < person->phoneCount; i++)
    {
      printf("%s", person->phoneNumbers[i]);
      if (i < person->phoneCount - 1)
        printf(", "); // Add a comma between items
    }
  }
  else
  {
    printf("N/A");
  }
  printf("\n");

  printf("%-15s : ", "Email Addresses");
  if (person->emailCount > 0 && person->emailAddresses)
  {
    for (int i = 0; i < person->emailCount; i++)
    {
      printf("%s", person->emailAddresses[i]);
      if (i < person->emailCount - 1)
        printf(", ");
    }
  }
  else
  {
    printf("N/A");
  }
  printf("\n");

  printf("%-15s : %s \n", "Married", person->isMarried ? "Yes" : "No");
  printf("%-15s : %s \n", "Employed", person->isEmployed ? "Yes" : "No");

  printf("\n\n");
}

// Main function to test the CRUD operations
int main()
{
  // Initialize the database from a JSON file
  initDatabase("database.json");

  // Test printing all persons in the database
  struct Person *current = persons;
  while (current != NULL)
  {
    printPerson(current);
    printf("\n");
    current = current->next;
  }

  // Test inserting a new person
  struct Person *newPerson = (struct Person *)malloc(sizeof(struct Person));
  newPerson->name = strdup("John Doe");
  newPerson->jobTitle = strdup("Software Engineer");
  newPerson->age = 30;
  newPerson->address = strdup("1234 Elm Street");
  newPerson->isMarried = false;
  newPerson->isEmployed = true;

  newPerson->phoneCount = 2;
  newPerson->phoneNumbers = (char **)malloc(newPerson->phoneCount * sizeof(char *));
  newPerson->phoneNumbers[0] = strdup("123-456-7890");
  newPerson->phoneNumbers[1] = strdup("098-765-4321");

  newPerson->emailCount = 1;
  newPerson->emailAddresses = (char **)malloc(newPerson->emailCount * sizeof(char *));
  newPerson->emailAddresses[0] = strdup("johndoe@example.com");

  insertPerson(newPerson);
  printf("Inserted new person:\n");
  printPerson(newPerson);

  // Test updating a person
  struct Person *updatedPerson = (struct Person *)malloc(sizeof(struct Person));
  updatedPerson->name = strdup("John Doe");
  updatedPerson->jobTitle = strdup("Senior Software Engineer");
  updatedPerson->age = 31;
  updatedPerson->address = strdup("1234 Oak Street");
  updatedPerson->isMarried = true;
  updatedPerson->isEmployed = true;

  updatedPerson->phoneCount = 1;
  updatedPerson->phoneNumbers = (char **)malloc(updatedPerson->phoneCount * sizeof(char *));
  updatedPerson->phoneNumbers[0] = strdup("111-222-3333");

  updatedPerson->emailCount = 2;
  updatedPerson->emailAddresses = (char **)malloc(updatedPerson->emailCount * sizeof(char *));
  updatedPerson->emailAddresses[0] = strdup("john.doe@newexample.com");
  updatedPerson->emailAddresses[1] = strdup("john.doe@anothermail.com");

  updatePerson("John Doe", updatedPerson);
  printf("\nUpdated person:\n");
  printPerson(findPerson("John Doe"));

  // Test deleting a person
  if (deletePerson("John Doe"))
  {
    printf("\nDeleted John Doe successfully.\n");
  }
  else
  {
    printf("\nFailed to delete John Doe.\n");
  }

  // Save the database back to the JSON file
  saveDatabase("updated_database.json");

  // Free all allocated memory before exiting
  current = persons;
  while (current != NULL)
  {
    struct Person *next = current->next;
    freePerson(current, false);
    current = next;
  }

  return 0;
}
