/*Функция сейчас не используется в коде программы. Оставленна на будущую доработку*/
/*Вывод содержимого директории*/
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

int print_dir(WINDOW * wnd, struct dirent **dir_entry, int *entries_amount_ptr, int relative_y) /*Получает имя директории*/
{
  int x, y;

  getyx(wnd, y, x);

  /*Вывод имён содержимого директории*/
   werase(wnd);
   wmove(wnd, 0, 0);
   for (int i = relative_y; i < (*entries_amount_ptr-relative_y); i++)
   {
     /*Если '.' или '..' - вывести описание их работы, а не точки*/
     if (strcmp(dir_entry[i]->d_name, ".") == 0)
     {
       wprintw(wnd, "/[CURRENT DIR]\n");
       continue;
     }
     if (strcmp(dir_entry[i]->d_name, "..") == 0)
     {
       wprintw(wnd, "/[PARENT DIR]\n");
       continue;
     }
     /*Иначе - просто вывести с пробелом перед именем*/
       wprintw(wnd, " %s\n", dir_entry[i]->d_name);
   }
   wmove(wnd, y, x);
  return 0; /*Вернуть указатель на структуру директории*/
}
