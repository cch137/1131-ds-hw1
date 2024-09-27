#ifndef CCH137_INTERFACE_H
#define CCH137_INTERFACE_H

#include "./cJSON.h"
#include "./database.h"

void print_person(DBItem *item);

// CRUD operations

void create_person(DBModel *person_model);
void read_person();
void update_person(DBModel *person_model);
void delete_person();

cJSON *input_cjson_with_model(DBModel *model, int depth);
cJSON *edit_cjson_with_model(DBModel *model, cJSON *json, int depth);

void main_menu();

#endif
