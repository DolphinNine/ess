//Функция расширения структуры и добавления новой записи в конец
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "proto.h"

struct phonebook *add_entry(struct phonebook *friends, char *pb_size_ptr, char s_name[5], unsigned int s_number)
{
  *pb_size_ptr+=1; //Увеличение объёма структуры на 1
  if ((friends = (struct phonebook *) realloc (friends, *pb_size_ptr * sizeof(struct phonebook))) == NULL) //Расширить объём структуры до нового размера
  {
    perror("Could not allocate reduced phonebook memory!");
    quit(friends, -1);
  }
  else
  {
    strcpy(friends[*pb_size_ptr-1].name, s_name);
    friends[*pb_size_ptr-1].number = s_number;
    return friends;
  }
}
