//Функция удаления записи по номеру и уменьшение общего размера структуры
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "proto.h"

struct phonebook *erase_entry(struct phonebook *friends, char *pb_size_ptr, int i)
{
  for (i = i-1; i < *pb_size_ptr-1; i++)
  {
    strcpy(friends[i].name, friends[i+1].name); //Перенос имени из следующей записи
    friends[i].number = friends[i+1].number; //Перенос номера телефона из следующей записи
  }
  *pb_size_ptr-=1; //Уменьшение объёма структуры на 1
  if ((friends = (struct phonebook *) realloc (friends, *pb_size_ptr * sizeof(struct phonebook))) == NULL) //Сжать объём структуры до нового размера
  {
    perror("Could not allocate reduced phonebook memory!\n");
    quit(friends, -1);
  }
  else
  {
    return friends;
  }
}
