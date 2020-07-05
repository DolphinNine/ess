//Функция поиска в структуре записи по номеру телефона
#include "defs.h"

int search_number(struct phonebook *friends, int pb_size, unsigned int s_number)
{
  for (int i = 0; i < pb_size; i++)
  {
    if (friends[i].number == s_number)
    {
      return i;
    }
  }
  return -1;
}
