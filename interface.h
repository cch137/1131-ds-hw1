#ifndef CCH137_INTERFACE_H
#define CCH137_INTERFACE_H

#include "./cJSON.h"
#include "./database.h"

void print_person(DBItem *item);

// CRUD operations

void create_person(DBModel *person_model);
void read_person();
void update_person();
void delete_person();

void main_menu();

#endif
