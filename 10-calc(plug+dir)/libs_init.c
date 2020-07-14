//Функция открытия библиотек-плагинов по вводимому имени файла
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include "defs.h"

/*Имя одной библиотеки - исключения.*/
/*Она уже загруженна на старте основной программы, и считываться снова не будет*/
#define MAIN_LIB "libcalc_lib.so"
#define LIBS_DIR "libs" /*Константа имени-пути директории библиотек*/

/*Функция фильтра*/
/*Поиск расширения библиотек .so, но отброс библиотеки MAIN_LIB*/
int filter(const struct dirent *libs_entry)
{
  const char *name;
  name = libs_entry->d_name; /*Забор имени*/
  char *type;
  type = strrchr(name, '.'); /*Указатель на символы после '.' - тип файла*/
  if((strcmp(type, ".so") == 0) && (strcmp(name, MAIN_LIB)) != 0)
  {
    /*Есть совпадение - значит это .so файл, но не MAIN_LIB*/
    return 1;
  }
  /*Нет совпадения - считанное имя не будет занесенно в структуру scandir-ом*/
  return 0;
}

/*Основная функция инициализации библиотек*/
struct list *libs_init(unsigned int *ls_size_ptr)
{
  char input[20]; //Хранение ввода с клавиатуры до помещения в структуру
  int (*curr_func)(struct list *libs, unsigned int i);
  struct dirent **libs_entry; /*Структура данных об элементах директории*/

  printf("\nInitializing libs. Looking up in ./%s dir\n", LIBS_DIR);
  /*Считывание директории и поиск там имён библиотек*/
  /*Читать, пока не дойдёт до конца или до ошибки*/
  /*Возвращаемое значение - число найденных записей, т.е. библиотек*/
  if ((*ls_size_ptr = scandir(LIBS_DIR, &libs_entry, filter, alphasort)) == -1)
  {
    perror("Could not scan the directory stream!");
    exit(-1);
  }

  /*Выделение памяти под собственную структуру данных*/
  /*Теперь достаточно одного выделения, без релоцирования под новые записи,
    так как итоговое число записей уже известно - оно было возвращено scandir*/
  if ((libs = (struct list *) malloc (*ls_size_ptr * sizeof(struct list))) == NULL) //Если память не выделилась - выйти
  {
    perror("Could not allocate initial libs list memory!");
    exit(-1);
  }

  //Открытие библиотек
  printf("Loaded libs:\n");
  for (int i = 0; i < *ls_size_ptr; i++)
  {
    strcpy(libs[i].path_name, libs_entry[i]->d_name); //Перенос ввода в ячейку путевого имени структуры
    libs[i].handle = dlopen(libs[i].path_name, RTLD_LAZY); //Забор хэндла
    curr_func = dlsym(libs[i].handle, "pass_name"); //Инициация функции передачи данных
    (*curr_func)(libs, i); //Запуск функции, с передачей ей указателя на структуру и индекса записи в ней.
    printf("[%d] - %s -> (%s)\n", i, libs[i].name, libs[i].path_name);
     /*пример из мануала вручную высвобождает память struct dirent* */
     /*Делается ручное высвобождение каждого эллемента, а затем всей структуры*/
    free(libs_entry[i]);
  }
  free(libs_entry);
  return libs;
}
