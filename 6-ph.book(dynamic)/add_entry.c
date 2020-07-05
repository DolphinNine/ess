//Функция расширения структуры и добавления новой записи в конец
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "proto.h"

struct phonebook *add_entry(struct phonebook *friends, int pb_size, char s_name[5], unsigned int s_number)
{
  pb_size++; //Увеличение объёма структуры на 1
  if ((friends = (struct phonebook *) realloc (friends, pb_size * sizeof(struct phonebook))) == NULL) //Расширить объём структуры до нового размера
  {
    perror("Could not allocate reduced phonebook memory!\n");
    quit(friends, -1);
  }
  else
  {
    strcpy(friends[pb_size-1].name, s_name);
    friends[pb_size-1].number = s_number;
    return friends;
  }
}
