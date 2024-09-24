#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./database.h"

char *inputLine();
char inputChar();
void printPerson(Person *person);
void mainMenu();

char *inputLine()
{
  char *buffer = NULL;
  size_t bufsize = 0;
  int length;

  length = getline(&buffer, &bufsize, stdin);

  if (length == -1)
  {
    perror("getline failed");
    return NULL;
  }

  // Remove the newline character from the buffer if it exists
  if (length > 0 && buffer[length - 1] == '\n')
  {
    buffer[length - 1] = '\0';
  }

  return buffer;
}

char inputChar()
{
  char *buffer = inputLine();
  char firstChar = buffer[0];
  free(buffer);
  return firstChar;
}

// Print a person’s information neatly
void printPerson(Person *person)
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
        printf(", ");
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

  printf("\n");
}

void mainMenu()
{
  while (1)
  {
    printf("======== Main Menu ========\n");
    printf("L  - Show namelist.\n");
    printf("A  - Add a new person.\n");
    printf("F  - Find person by name.\n");
    printf("U  - Update a person.\n");
    printf("D  - Delete person by name.\n");
    printf("X  - Exit.\n");
    printf("\nEnter your choice: ");

    switch (inputChar())
    {
    case 'L':
    case 'l':
    {
      printf("Show namelist.\n");
      Person *cursor = personsHead;
      while (cursor != NULL)
      {
        printf("- %s\n", cursor->name);
        cursor = cursor->next;
      }
      break;
    }

    case 'A':
    case 'a':
    {
      Person *newPerson = (Person *)malloc(sizeof(Person));
      if (newPerson == NULL)
      {
        perror("Failed to allocate memory for new person");
        return;
      }
      memset(newPerson, 0, sizeof(Person)); // Initialize memory to zero
      char *buffer;
      int phoneCount, emailCount;

      // Input name
      printf("Enter name: ");
      buffer = inputLine();
      while (buffer && exists(buffer))
      {
        printf("The name already exists. Enter name: ");
        buffer = inputLine();
      }
      if (buffer)
      {
        newPerson->name = strdup(buffer);
        free(buffer);
      }

      // Input job title
      printf("Enter job title: ");
      buffer = inputLine();
      if (buffer)
      {
        newPerson->jobTitle = strdup(buffer);
        free(buffer);
      }

      // Input age
      printf("Enter age: ");
      while (scanf("%d", &newPerson->age) != 1 || newPerson->age < 0 || newPerson->age > 150)
      {
        printf("Invalid input. Enter a valid age (0-150): ");
        scanf("%*[^\n]");
        scanf("%*c");
      }
      getchar(); // Consume newline left by scanf

      // Input address
      printf("Enter address: ");
      buffer = inputLine();
      if (buffer)
      {
        newPerson->address = strdup(buffer);
        free(buffer);
      }

      // Collect phone numbers
      printf("How many phone numbers? ");
      while (scanf("%d", &phoneCount) != 1 || phoneCount < 0 || phoneCount > 10)
      {
        printf("Invalid input. Enter a valid number of phone numbers (0-10): ");
        scanf("%*[^\n]");
        scanf("%*c");
      }
      getchar(); // Consume newline left by scanf
      newPerson->phoneCount = phoneCount;
      newPerson->phoneNumbers = malloc(phoneCount * sizeof(char *));
      for (int i = 0; i < phoneCount; i++)
      {
        printf("Enter phone number %d: ", i + 1);
        buffer = inputLine();
        if (buffer)
        {
          newPerson->phoneNumbers[i] = strdup(buffer);
          free(buffer);
        }
      }

      // Collect email addresses
      printf("How many email addresses? ");
      while (scanf("%d", &emailCount) != 1 || emailCount < 0 || emailCount > 10)
      {
        printf("Invalid input. Enter a valid number of email addresses (0-10): ");
        scanf("%*[^\n]");
        scanf("%*c");
      }
      getchar();
      newPerson->emailCount = emailCount;
      newPerson->emailAddresses = malloc(emailCount * sizeof(char *));
      for (int i = 0; i < emailCount; i++)
      {
        printf("Enter email address %d: ", i + 1);
        buffer = inputLine();
        if (buffer)
        {
          newPerson->emailAddresses[i] = strdup(buffer);
          free(buffer);
        }
      }

      // Married and employed status
      printf("Is the person married? (1 for Yes, 0 for No): ");
      while (scanf("%d", &newPerson->isMarried) != 1 || (newPerson->isMarried != 0 && newPerson->isMarried != 1))
      {
        printf("Invalid input. Enter 1 for Yes, 0 for No: ");
        scanf("%*[^\n]");
        scanf("%*c");
      }

      printf("Is the person employed? (1 for Yes, 0 for No): ");
      while (scanf("%d", &newPerson->isEmployed) != 1 || (newPerson->isEmployed != 0 && newPerson->isEmployed != 1))
      {
        printf("Invalid input. Enter 1 for Yes, 0 for No: ");
        scanf("%*[^\n]");
        scanf("%*c");
      }

      int createCode = createPerson(newPerson);
      if (createCode)
      {
        printf("Failed to add person. (Error code: %d)\n", createCode);
        free(newPerson);
      }
      else
      {
        printf("Person added successfully.\n");
        printPerson(newPerson);
      }

      break;
    }

    case 'F':
    case 'f':
    {
      char *name = NULL;
      printf("Enter name to search: ");
      name = inputLine();
      if (name)
      {
        Person *foundPerson = findPerson(name);
        if (foundPerson)
        {
          printf("Person found:\n");
          printPerson(foundPerson);
        }
        else
        {
          printf("Person not found.\n");
        }
        free(name);
      }
      break;
    }

    case 'U':
    case 'u':
    {
      char *name = NULL;
      printf("Enter name to update: ");
      name = inputLine();
      if (name)
      {
        Person *foundPerson = findPerson(name);
        if (foundPerson)
        {
          if (updatePerson(name, foundPerson))
          {
            printf("Person updated successfully.\n");
          }
          else
          {
            printf("Failed to update person.\n");
          }
        }
        else
        {
          printf("Person not found.\n");
        }
        free(name);
      }
      break;
    }

    case 'D':
    case 'd':
    {
      char *name = NULL;
      printf("Enter name to delete: ");
      name = inputLine();
      if (name)
      {
        if (deletePerson(name))
        {
          printf("Person deleted successfully.\n");
        }
        else
        {
          printf("Failed to delete person.\n");
        }
        free(name);
      }
      break;
    }

    case 'X':
    case 'x':
      printf("Exiting program...\n");
      return;

    default:
      printf("Invalid choice, please try again.\n");
    }

    printf("\n");
  }
}
