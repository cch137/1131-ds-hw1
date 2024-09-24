#ifndef CCH137_DATABASE_H
#define CCH137_DATABASE_H

#include <stdint.h>
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

extern Person *personsHead;

Person *personConstructor(cJSON *personJSON);
cJSON *personJSONConstructor(Person *person);
void clearPerson(Person *person);
void clearDatabase();
void initDatabase(const char *filename);
void saveDatabase(const char *filename);

Person *findPerson(const char *name);
uint8_t createPerson(Person *newPerson);
bool updatePerson(const char *name, Person *updatedPerson);
bool deletePerson(const char *name);
bool exists(const char *name);

#endif
