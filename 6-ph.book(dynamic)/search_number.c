//Функция поиска в структуре записи по номеру телефона
#include "defs.h"

int search_number(struct phonebook *friends, char *pb_size_ptr, unsigned int s_number)
{
  for (int i = 0; i < *pb_size_ptr; i++)
  {
    if (friends[i].number == s_number)
    {
      return i;
    }
  }
  return -1;
}
