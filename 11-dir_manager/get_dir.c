/*Забор содержимого директории*/
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

struct dirent **get_dir(WINDOW * wnd, struct dirent **dir_entry, int *entries_amount_ptr, char dir_name[256]) /*Получает имя директории*/
{
  int mx_std, my_std; /*Хранение размеров основного окна*/
  /*Используется scandir для вывода именён в нужном порядке(по алфавиту).
  Так '.' и '..' будут идти раньше всех*/
  if ((*entries_amount_ptr = scandir(dir_name, &dir_entry, NULL, alphasort)) == -1)
  {
    /*-1 - не обязательно ошибка*/
    /*Возможно, это не директория?*/
    if (errno == ENOTDIR)
    {
      attron(COLOR_PAIR(3)); /*Использовать красный цвет выделения текста*/
      getmaxyx(stdscr, my_std, mx_std);
      mvprintw(my_std-1, 2, "NOT A DIRECTORY!"); /*Сообщить об этом на дне главного окна*/
      refresh();
      attron(COLOR_PAIR(2)); /*Вернуть цвет*/
      /*Нет смысла выводить НЕ директорию. Преждевременный выход из функции*/
      return dir_entry;
    }
    /*Если это иная ошибка - выйти из программы*/
    endwin();
    perror("Could not scan the directory stream!");
    exit(-1);
  }
  /*Вывод имён содержимого директории*/
   werase(wnd);
   wmove(wnd, 0, 0);
   for (int i = 0; i < *entries_amount_ptr; i++)
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
  return dir_entry; /*Вернуть указатель на структуру директории*/
}
