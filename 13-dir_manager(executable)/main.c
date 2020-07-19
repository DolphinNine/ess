/*Основная программа обозревателя директорий*/
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "proto.h"

//#define PATH_MAX 4096 /*Значение уже было объявленно в usr/include/linux/limits.h*/

struct winsize size;

 /* (функция из статьи, меняющая размер окна (или скорее мешающая вылету)) */
void sig_winch()
{
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
  resizeterm(size.ws_row, size.ws_col);
  //getmaxyx(wnd_left, my, mx); //(нужно добавить изменение макс.размеров, при изменении окна)
}

int main()
{
  WINDOW *wnd_left, *wnd_right; //Указатель на окно
  WINDOW **active_wnd = &wnd_left; /*Указатель на активное сейчасо окно*/

  int i, j; /*Итерации*/
  int x, y, mx, my, mx_std, my_std; /*Координаты и их максимальные значения*/
  /*Значения числа записей и относительной координаты для разных окон*/
  int entries_amount_left, relative_y_left, entries_amount_right, relative_y_right;
  int *entries_amount_ptr; /*Указатель на число записей в директории*/
  int *relative_y_ptr; /*Укзатель числа "лишних" строк*/

  char ch; //Хранение считанного символа
  char cwd_left[PATH_MAX], cwd_right[PATH_MAX]; /*Хранение пути текущей директории*/
  char *active_cwd = cwd_left;

  struct dirent **dir_entry_left, **dir_entry_right;/*структура данных о директории*/
  struct dirent ***dir_entry_ptr = &dir_entry_left;

  initscr();
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size); //Вызов функции для определения текущего размера экрана.
  signal(SIGWINCH, sig_winch); //Обработка изменения размера экрана
  curs_set(1); //Установка режима курсора
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLUE); //Пара для окна
  init_pair(2, COLOR_BLACK, COLOR_WHITE); //Пара для основоного экрана
  init_pair(3, COLOR_WHITE, COLOR_RED); /*Пара для сообщений*/

  attron(COLOR_PAIR(2));
  bkgd(COLOR_PAIR(2));
  mvprintw(0, 2, "Enter - Open | Tab - Switch window | F12 - Exit");

  //Открытие нового окна и установка его режимов
  wnd_left = draw_win(wnd_left, 1, 2);
  wnd_right = draw_win(wnd_right, 1, ((size.ws_col)/2)+1);

  /*Заранее запрашиваются макс.размеры окон */
  getmaxyx(stdscr, my_std, mx_std);
  getmaxyx(wnd_left, my, mx); /*Должны быть идентичны для обоих окон*/
  /*Уменьшить на единицу для дальнейшего удобства пользования*/
  my--;
  mx--;
  my_std--;
  mx_std--;
  cbreak();
  noecho();
  wmove(*active_wnd, 0, 0);

  /*Прежде, чем можно будет вызвать функцию чтения и печати директории
    Нужно позабоиться о передачи функции указателей.
    Для этого придётся вручную, по очереди, передать указателям адреса каждого
    из окон, а затем вызвать функцию.*/
  dir_entry_ptr = &dir_entry_right; /*Указатель структуры директории*/
  entries_amount_ptr = &entries_amount_right; /*..числа записей*/
  relative_y_ptr = &relative_y_right; /*..относительной координаты*/
  /*Вызов собственной функции считывания и вывода директории.
  Начало - в стартовой папке программы.*/
  dir_entry_right = get_req(wnd_right, *dir_entry_ptr, entries_amount_ptr, relative_y_ptr, ".");

  dir_entry_ptr = &dir_entry_left;
  entries_amount_ptr = &entries_amount_left;
  relative_y_ptr = &relative_y_left;
  dir_entry_left = get_req(wnd_left, *dir_entry_ptr, entries_amount_ptr, relative_y_ptr, ".");

  /*Запомнить текущие директории для каждого окна*/
  getcwd(cwd_left, PATH_MAX);
  getcwd(cwd_right, PATH_MAX);
  wrefresh(wnd_left);
  wrefresh(wnd_right);
  wmove(*active_wnd, *entries_amount_ptr-1, 0);

  /*Бесконечный цикл обработки команд*/
  while(1)
  {
    /* Любой символ считывается с клавиатуры.*/
    /*Если это не команда - он просто выводится на экран в конце*/
    ch = wgetch(*active_wnd);
    /*Обновление основного окна убирает сообщения на его дне,
      относящиеся к предыдущему шагу*/
    refresh();
    switch (ch) /*Если символ командный - он будет обработан здесь*/
    {

/*Клавиши*/
/*---------------------------------------------------------------------------*/
/*Вверх*/
      case (char)KEY_UP:
        getyx(*active_wnd, y, x);
        /*Если не край экрана - просто сдвинуться*/
        if ((y - 1) >= 0)
        {
          wmove(*active_wnd, y-1, x);
        }
        /*Иначе - сдвинуть сам экран*/
        else
        {
        /*Двигать только, если есть зачем - то есть не был достигнут предел
        записей в директории*/
          if (*relative_y_ptr > 0)
          {
            (*relative_y_ptr)--;
            wmove(*active_wnd, 0, 1);
            wscrl(*active_wnd, -1);
            /*Вывод делается здесь же.
            Возможно было бы использование отдельной функции (например, print_dir с модификацией) -
            Это бы позволило форматировать вывод нужным образом уже там, не вставляя его сюда,
            но пока что это опущено.*/
            /*Вывод не будет особым образом обрабатывать '.' и '..',
            и они будут выведенны, как есть*/
            wprintw(*active_wnd, "%s", (*dir_entry_ptr)[*relative_y_ptr]->d_name);
            wmove(*active_wnd, 0, 0);
          }
        }
      break;

/*---------------------------------------------------------------------------*/
/*Вниз*/
      /*Работает идентично вверху, только наоборот*/
      case (char)KEY_DOWN:
        getyx(*active_wnd, y, x);
        if ((y + 1) < my)
        {
          /*Дополнительное ограничение, чтобы не листать ниже общего числа записей*/
          if ((y + 1) < *entries_amount_ptr)
          {
            wmove(*active_wnd, y+1, x);
          }
        }
        else
        {
          if ((*relative_y_ptr + 1) <= *entries_amount_ptr - my)
          {
            (*relative_y_ptr)++;
            wmove(*active_wnd, my, 1);
            wscrl(*active_wnd, 1);
            if((my + *relative_y_ptr) != *entries_amount_ptr)
            {
              wprintw(*active_wnd, "%s", (*dir_entry_ptr)[my + *relative_y_ptr]->d_name);
              wmove(*active_wnd, my, 0);
            }
            else
            {
              wmove(*active_wnd, my - 1, 0);
            }
          }
        }
      break;

/*---------------------------------------------------------------------------*/
/*Enter*/
      case 10 : /*(char)KEY_ENTER ~ 10*/
        getyx(*active_wnd, y, x); /*Считать позицию нажатия*/
        /*y соответствует номеру записи в структуре*/
        /*Номер записи используется для получения и передачи имени директории*/
        strcpy(active_cwd, (*dir_entry_ptr)[y + *relative_y_ptr]->d_name); /*Запомнить директорию*/
        *dir_entry_ptr = get_req(*active_wnd, *dir_entry_ptr, entries_amount_ptr, relative_y_ptr, (*dir_entry_ptr)[y + *relative_y_ptr]->d_name);
        /*Если не сменить директорию - все последующие чтения директорий
        будут идти из стартовой '.', а там нужный папок не окажется*/
        chdir(active_cwd);
        /*(стоит дописать тут сохранение позиции курсора так, чтобы он
          не попал на незаполненные строки. пока что, позиция, по сути,
          сбрасывается на дно списка)*/
        if (*relative_y_ptr == 0)
        {
          wmove(*active_wnd, *entries_amount_ptr - 1, 0);
        }
        else
        {
          wmove(*active_wnd, y, x);
        }
      break;

/*---------------------------------------------------------------------------*/
/*Tab*/
      /*Функция - смена активного окна на противоположное*/
      case '\t':
        getcwd(active_cwd, PATH_MAX);
        /*Перенесенние указателей на данные нового активного окна*/
        /*(можно было бы составить стркутуру, и копировать её?)*/
        if (*active_wnd == wnd_left)
        {
          active_wnd = &wnd_right;
          entries_amount_ptr = &entries_amount_right;
          relative_y_ptr = &relative_y_right;
          active_cwd = cwd_right;
          dir_entry_ptr = &dir_entry_right;
        }
        else
        {
          active_wnd = &wnd_left;
          entries_amount_ptr = &entries_amount_left;
          relative_y_ptr = &relative_y_left;
          active_cwd = cwd_left;
          dir_entry_ptr = &dir_entry_left;
        }
        chdir(active_cwd); /*Смена директорию на активную в окне*/
        if (*relative_y_ptr == 0)
        {
          wmove(*active_wnd, *entries_amount_ptr - 1, 0);
        }
        else
        {
          wmove(*active_wnd, my - 1, 0);
        }
      break;

/*---------------------------------------------------------------------------*/
/*F12*/
      //Выйти из программы
      case (char)KEY_F(12):
        quit(*dir_entry_ptr, entries_amount_ptr, 0);
      break;

      /*Ничего не произошло - значит клавиша не была обработываемой командой*/
      /*В редакторе имело смысл выводить клавишу на экран, но тут это будет мешать*/
      default:
        //wechochar(*active_wnd, (char)ch);
      break;
    }
    getyx(*active_wnd, y, x);
    move(my_std, 0);
    clrtoeol();
    wmove(*active_wnd, y, x);
    wrefresh(*active_wnd);
  }
}
