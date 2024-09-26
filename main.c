#include "./cJSON.h"
#include "./database.h"
#include "./interface.h"

int main()
{
  init_database("database.json");
  main_menu();
  save_database("database.json");

  return 0;
}
