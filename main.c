#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./cJSON.h"
#include "./database.h"
#include "./menu.h"

// Main function to test the CRUD operations
int main()
{
  // Initialize the database from a JSON file
  initDatabase("database.json");

  mainMenu();

  // Save the database back to the JSON file
  saveDatabase("database.json");
  clearDatabase();

  return 0;
}
