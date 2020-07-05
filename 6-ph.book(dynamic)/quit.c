//Функция выхода с очисткой памяти.
//Сейчас, при любом исходе меняется только возвращаемое значение
#include <stdlib.h>

int quit(struct phonebook *friends, int state)
{
  if (state == 0)
  {
    free(friends); //Высвобождение динамической памяти
    exit(0);
  }
  else
  {
    free(friends); //Высвобождение динамической памяти
    exit(-1);
  }
}
