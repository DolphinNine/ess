//Функция вывода всей структуры.
//Работает с выводом сама, напрямую
#include <stdio.h>
#include "defs.h"

void show_all(struct phonebook *friends, int pb_size)
{
  for (int i = 0; i < pb_size; i++)
  {
    printf("[%d] %s's tel.: %d\n", i+1, friends[i].name, friends[i].number);
  }
}
