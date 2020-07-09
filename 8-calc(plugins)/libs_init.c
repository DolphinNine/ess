//Функция открытия библиотек-плагинов по вводимому имени файла
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "defs.h"

struct list *libs_init(unsigned int *ls_size_ptr)
{
  char input[20]; //Хранение ввода с клавиатуры до помещения в структуру
  int (*curr_func)(struct list *libs, unsigned int i);

  if ((libs = (struct list *) malloc (*ls_size_ptr * sizeof(struct list))) == NULL) //Если память не выделилась - выйти
  {
    perror("Could not allocate initial libs list memory!");
    //return -1;
  }
  //Заполнение поля пути в структуре с помощью ввода от пользователя
  printf("\nInitializing libs. Names goes like lib'operation'.so\n");
  while (1)
  {
    printf("One library full name (or [n] to stop): ");
    scanf("%s", input);
    if(input[0] == 'n') {break;} //Если пустой ввод(enter) - выйти из бесконечного цикла while
    strcpy(libs[*ls_size_ptr-1].path_name, input); //Перенос ввода в ячейку путевого имени структуры
    *ls_size_ptr+=1; //Число всегда будет на единицу больше реального числа записей с библиотеками
    if ((libs = (struct list *) realloc (libs, *ls_size_ptr * sizeof(struct list))) == NULL) //Если память не выделилась - выйти
    {
      perror("Could not rellocate libs list memory!");
      //return -1;
    }
  }
  //Открытие библиотек
  for (int i = 0; i < *ls_size_ptr-1; i++) //ls_size всё ещё больше на единицу, чем нужно, потому уменьшается
  {
    libs[i].handle = dlopen(libs[i].path_name, RTLD_LAZY); //Забор хэндла
    curr_func = dlsym(libs[i].handle, "pass_name"); //Инициация функции передачи данных
    (*curr_func)(libs, i); //Запуск функции, с передачей ей указателя на структуру и индекса записи в ней.
  }
  return libs;
}
