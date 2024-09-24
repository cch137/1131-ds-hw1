#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./cJSON.h"

// Global variables
struct Person *persons = NULL;
int personCount = 0;

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
};

// Function prototypes
struct Person *createPersonFromJSON(cJSON *json);
void freePerson(struct Person *p);
void parseJSONFile(const char *filename);
void writeJSONFile(const char *filename);
void findPerson(const char *name);
void insertPerson(struct Person newPerson);
void updatePerson(const char *name, struct Person updatedPerson);
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
        if (p->phoneNumbers == NULL)
        {
            printf("Memory allocation for phoneNumbers failed\n");
            free(p);
            exit(1);
        }
        for (int i = 0; i < p->phoneCount; ++i)
        {
            cJSON *phoneItem = cJSON_GetArrayItem(phoneArray, i);
            p->phoneNumbers[i] = phoneItem ? strdup(phoneItem->valuestring) : NULL;
        }
    }
    else
    {
        p->phoneCount = 0;
        p->phoneNumbers = NULL;
    }

    cJSON *emailArray = cJSON_GetObjectItem(json, "emailAddresses");
    if (emailArray)
    {
        p->emailCount = cJSON_GetArraySize(emailArray);
        p->emailAddresses = (char **)malloc(p->emailCount * sizeof(char *));
        if (p->emailAddresses == NULL)
        {
            printf("Memory allocation for emailAddresses failed\n");
            free(p->phoneNumbers);
            free(p);
            exit(1);
        }
        for (int i = 0; i < p->emailCount; ++i)
        {
            cJSON *emailItem = cJSON_GetArrayItem(emailArray, i);
            p->emailAddresses[i] = emailItem ? strdup(emailItem->valuestring) : NULL;
        }
    }
    else
    {
        p->emailCount = 0;
        p->emailAddresses = NULL;
    }

    return p;
}

// Free memory allocated for a Person
void freePerson(struct Person *p)
{
    if (p == NULL)
        return;

    // Free dynamically allocated strings and arrays if they exist
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
}

// Parse JSON file and initialize an array of Person structs
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
    if (data == NULL)
    {
        printf("Memory allocation failed\n");
        fclose(file);
        exit(1);
    }
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

    personCount = cJSON_GetArraySize(personsArray);
    persons = (struct Person *)malloc(personCount * sizeof(struct Person));
    if (persons == NULL)
    {
        printf("Memory allocation for persons array failed\n");
        cJSON_Delete(json);
        exit(1);
    }

    for (int i = 0; i < personCount; ++i)
    {
        cJSON *personJSON = cJSON_GetArrayItem(personsArray, i);
        if (personJSON == NULL)
        {
            printf("Error: personJSON is NULL at index %d\n", i);
            cJSON_Delete(json);
            free(persons);
            exit(1);
        }
        struct Person *person = createPersonFromJSON(personJSON);
        persons[i] = *person;
        free(person); // Memory is copied, so free temporary struct
    }

    cJSON_Delete(json);
}

// Write the updated Person array to a JSON file
void writeJSONFile(const char *filename)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *personsArray = cJSON_CreateArray();

    for (int i = 0; i < personCount; ++i)
    {
        struct Person *p = &persons[i];
        cJSON *personJSON = cJSON_CreateObject();
        cJSON_AddStringToObject(personJSON, "name", p->name ? p->name : "");
        cJSON_AddStringToObject(personJSON, "jobTitle", p->jobTitle ? p->jobTitle : "");
        cJSON_AddNumberToObject(personJSON, "age", p->age);
        cJSON_AddStringToObject(personJSON, "address", p->address ? p->address : "");

        cJSON *phoneArray = cJSON_CreateArray();
        for (int j = 0; j < p->phoneCount; ++j)
        {
            cJSON_AddItemToArray(phoneArray, cJSON_CreateString(p->phoneNumbers[j] ? p->phoneNumbers[j] : ""));
        }
        cJSON_AddItemToObject(personJSON, "phoneNumbers", phoneArray);

        cJSON *emailArray = cJSON_CreateArray();
        for (int j = 0; j < p->emailCount; ++j)
        {
            cJSON_AddItemToArray(emailArray, cJSON_CreateString(p->emailAddresses[j] ? p->emailAddresses[j] : ""));
        }
        cJSON_AddItemToObject(personJSON, "emailAddresses", emailArray);

        cJSON_AddBoolToObject(personJSON, "isMarried", p->isMarried);
        cJSON_AddBoolToObject(personJSON, "isEmployed", p->isEmployed);

        cJSON_AddItemToArray(personsArray, personJSON);
    }

    cJSON_AddItemToObject(root, "persons", personsArray);
    char *jsonData = cJSON_Print(root);
    if (jsonData == NULL)
    {
        printf("Error generating JSON data\n");
        cJSON_Delete(root);
        exit(1);
    }
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error opening file for writing %s\n", filename);
        cJSON_Delete(root);
        free(jsonData);
        exit(1);
    }
    fwrite(jsonData, 1, strlen(jsonData), file);
    fclose(file);
    cJSON_Delete(root);
    free(jsonData);
}

// Find a person by name
void findPerson(const char *name)
{
    for (int i = 0; i < personCount; ++i)
    {
        if (strcmp(persons[i].name, name) == 0)
        {
            struct Person *p = &persons[i];
            printf("Found Person:\n");
            printf("Name: %s\n", p->name);
            printf("Job Title: %s\n", p->jobTitle);
            printf("Age: %d\n", p->age);
            printf("Address: %s\n", p->address);
            printf("Phone Numbers:\n");
            for (int j = 0; j < p->phoneCount; ++j)
            {
                printf("  %s\n", p->phoneNumbers[j]);
            }
            printf("Email Addresses:\n");
            for (int j = 0; j < p->emailCount; ++j)
            {
                printf("  %s\n", p->emailAddresses[j]);
            }
            printf("Married: %s\n", p->isMarried ? "Yes" : "No");
            printf("Employed: %s\n", p->isEmployed ? "Yes" : "No");
            return;
        }
    }
    printf("Person with name %s not found\n", name);
}

// Insert a new person
void insertPerson(struct Person newPerson)
{
    persons = (struct Person *)realloc(persons, (personCount + 1) * sizeof(struct Person));
    if (persons == NULL)
    {
        printf("Memory reallocation failed\n");
        exit(1);
    }
    persons[personCount] = newPerson;
    personCount++;
}

// Update an existing person
void updatePerson(const char *name, struct Person updatedPerson)
{
    for (int i = 0; i < personCount; ++i)
    {
        if (strcmp(persons[i].name, name) == 0)
        {
            freePerson(&persons[i]);

            persons[i].name = strdup(updatedPerson.name);
            persons[i].jobTitle = strdup(updatedPerson.jobTitle);
            persons[i].age = updatedPerson.age;
            persons[i].address = strdup(updatedPerson.address);

            persons[i].phoneCount = updatedPerson.phoneCount;
            persons[i].phoneNumbers = (char **)malloc(updatedPerson.phoneCount * sizeof(char *));
            for (int j = 0; j < updatedPerson.phoneCount; ++j)
            {
                persons[i].phoneNumbers[j] = strdup(updatedPerson.phoneNumbers[j]);
            }

            persons[i].emailCount = updatedPerson.emailCount;
            persons[i].emailAddresses = (char **)malloc(updatedPerson.emailCount * sizeof(char *));
            for (int j = 0; j < updatedPerson.emailCount; ++j)
            {
                persons[i].emailAddresses[j] = strdup(updatedPerson.emailAddresses[j]);
            }

            persons[i].isMarried = updatedPerson.isMarried;
            persons[i].isEmployed = updatedPerson.isEmployed;

            return;
        }
    }
    printf("Person with name %s not found\n", name);
}

// Delete a person by name
void deletePerson(const char *name)
{
    for (int i = 0; i < personCount; ++i)
    {
        if (strcmp(persons[i].name, name) == 0)
        {
            freePerson(&persons[i]);
            for (int j = i; j < personCount - 1; ++j)
            {
                persons[j] = persons[j + 1];
            }
            persons = (struct Person *)realloc(persons, (personCount - 1) * sizeof(struct Person));
            if (persons == NULL && personCount > 1)
            {
                printf("Memory reallocation failed\n");
                exit(1);
            }
            personCount--;
            return;
        }
    }
    printf("Person with name %s not found\n", name);
}

// Main function
int main()
{
    parseJSONFile("database.json");

    // Example CRUD operations
    printf("Find person 'Alice':\n");
    findPerson("Alice");

    printf("\nInserting a new person:\n");
    struct Person newPerson = {
        .name = strdup("Charlie"),
        .jobTitle = strdup("UX Designer"),
        .age = 28,
        .address = strdup("789 Maple St"),
        .phoneNumbers = (char **)malloc(1 * sizeof(char *)),
        .phoneCount = 1,
        .emailAddresses = (char **)malloc(1 * sizeof(char *)),
        .emailCount = 1,
        .isMarried = false,
        .isEmployed = true};
    newPerson.phoneNumbers[0] = strdup("555-987-6543");
    newPerson.emailAddresses[0] = strdup("charlie@example.com");
    insertPerson(newPerson);

    printf("\nUpdating person 'Bob':\n");
    struct Person updatedPerson = {
        .name = strdup("Bob"),
        .jobTitle = strdup("Senior Data Scientist"),
        .age = 26,
        .address = strdup("456 Elm St, Apt 2"),
        .phoneNumbers = (char **)malloc(1 * sizeof(char *)),
        .phoneCount = 1,
        .emailAddresses = (char **)malloc(1 * sizeof(char *)),
        .emailCount = 1,
        .isMarried = false,
        .isEmployed = true};
    updatedPerson.phoneNumbers[0] = strdup("555-765-4321");
    updatedPerson.emailAddresses[0] = strdup("bob.updated@example.com");
    updatePerson("Bob", updatedPerson);

    printf("\nDeleting person 'Alice':\n");
    deletePerson("Alice");

    // Write updated data to JSON file
    writeJSONFile("database.json");

    // Free allocated memory
    for (int i = 0; i < personCount; ++i)
    {
        freePerson(&persons[i]);
    }
    free(persons);

    return 0;
}
