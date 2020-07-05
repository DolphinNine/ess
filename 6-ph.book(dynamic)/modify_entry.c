//Функция изменения записи по номеру, перезаписывающая имя и номер телефона
#include <string.h>
#include "defs.h"

int modify_entry(struct phonebook *friends, int i, char s_name[5], unsigned int s_number)
{
  strcpy(friends[i-1].name, s_name);
  friends[i-1].number = s_number;
  return 0;
}
