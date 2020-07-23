/*Функция создания и отрисовки окна*/
#include <sys/ioctl.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>


WINDOW *draw_win(WINDOW *wnd, int pos_y, int pos_x)
{
  struct winsize size;

  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size); //Вызов функции для определения текущего размера экрана.
  curs_set(1); //Установка режима курсора

  //Открытие нового окна и установка его режимов
  wnd = newwin((size.ws_row-2), ((size.ws_col)/2)-3, pos_y, pos_x);
  wattron(wnd, COLOR_PAIR(2));
  wbkgd(wnd, COLOR_PAIR(1));
  keypad(wnd, TRUE); //Передавать программе коды спец.клавиш, вместо esc seq.
  /*Включение печати содержимого за пределами видимого экрана.
  Проще говоря - скроллинг.*/
  idlok(wnd, TRUE);
  scrollok(wnd, TRUE);

  refresh();
  wrefresh(wnd);

  return wnd;
}
