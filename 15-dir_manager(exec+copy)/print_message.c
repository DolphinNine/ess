/*Напечатать сообщение внизу главного экрана*/
#include <curses.h>

int print_message(WINDOW *wnd, int message_type, char message_text[50])
{
  int mx_std, my_std, y, x;
  char mode;

  /*Определение цвета сообщения по типу сообщения, полученного извне*/
  if(message_type > 0) /*Зелёный цвет*/
  {
    mode = 4;
  }
  else
  {
    if(message_type < 0) /*Красный цвет*/
    {
      mode = 3;
    }
    else /*Белый цвет*/
    {
      mode = 2;
    }
  }
  getmaxyx(stdscr, my_std, mx_std); /*Получить размеры главного окна*/
  getyx(wnd, y, x); /*Получить текущую позицию курсора активного окна*/
  attron(COLOR_PAIR((int)mode)); /*Использовать установленный ранее цвет*/
  mvprintw(my_std-1, 2, message_text); /*Вывести сообщение на дне главного окна*/
  refresh();
  attron(COLOR_PAIR(2)); /*Вернуть цвет на обычный(белый)*/
  wmove(wnd, y, x); /*Вернуть кусор в активное окно на старую позицию*/
  return 0;
}
