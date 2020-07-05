//Функция поиска в структуре записи по имени
#include <string.h>
#include "defs.h"

int search_name(struct phonebook *friends, int pb_size, char s_name[5])
{
  for (int i = 0; i < pb_size; i++)
  {
    if (strcmp(friends[i].name, s_name) == 0)
    {
      return i;
    }
  }
  return -1;
}
