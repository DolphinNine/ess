//Функция поиска в структуре записи по имени
#include <string.h>
#include "defs.h"

int search_name(struct phonebook *friends, char *pb_size_ptr, char s_name[5])
{
  for (int i = 0; i < *pb_size_ptr; i++)
  {
    if (strcmp(friends[i].name, s_name) == 0)
    {
      return i;
    }
  }
  return -1;
}
