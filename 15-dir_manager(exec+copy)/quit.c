/*Простая функция очистки памяти, удаления файлов и выхода и программы*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <dirent.h>

int quit(struct dirent **dir_entry, int *entries_amount_ptr, int state)
{
  endwin(); /*Закрыть окно ncurses*/
  /*Цикл очистки памяти данных о директории от scandir*/
  for (int i = 0; i < *entries_amount_ptr; i++)
  {
    free(dir_entry[i]);
  }
  free(dir_entry);
  remove("/tmp/source.wnd"); /*Удалить файл восстановления окна*/
  exit(state); /*Выйти*/
}
