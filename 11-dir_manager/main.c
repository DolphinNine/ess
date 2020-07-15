/*Основная программа обозревателя директорий*/
/*Работает в режиме одного окна. Второе пока не реализовано*/
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "proto.h"

#define BUF_SIZE 100

struct winsize size;

 /* (функция из статьи, меняющая размер окна (или скорее мешающая вылету)) */
void sig_winch()
{
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
  resizeterm(size.ws_row, size.ws_col);
  //getmaxyx(wnd_left, my, mx); //(нужно добавить изменение макс.размеров, при изменении окна)
}

int main() // (стоит позже переписать на получение имени)
{
  WINDOW * wnd_left; //Указатель на окно
  int i, j, x, y, mx, my, mx_std, my_std, entries_amount, relative_y;
  int *entries_amount_ptr = &entries_amount; /*Указатель на число записей в директории*/
  char ch; //Хранение считанного символа
  char last_dir_name[256]; /*Имя директории, которое было отдано get_dir.*/
  struct dirent **dir_entry;/*структура данных о директории*/

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
  printw("  Enter - Open | F12 - Exit");

  //Открытие нового окна и установка его режимов
  wnd_left = newwin(size.ws_row-2, size.ws_col-4, 1, 2);
  wattron(wnd_left, COLOR_PAIR(2));
  wbkgd(wnd_left, COLOR_PAIR(1));
  getmaxyx(stdscr, my_std, mx_std);
  getmaxyx(wnd_left, my, mx); /*Заранее запрашиваются макс.размеры окна */
  /*Уменьшить на единицу для дальнейшего удобства пользования*/
  my--;
  mx--;
  my_std--;
  mx_std--;
  keypad(wnd_left, TRUE); //Передавать программе коды спец.клавиш, вместо esc seq.
  cbreak();
  noecho();
  /*Включение печати содержимого за пределами видимого экрана.
  Проще говоря - скроллинг.*/
  idlok(wnd_left, TRUE);
  scrollok(wnd_left, TRUE);

  refresh();
  wrefresh(wnd_left);
  wmove(wnd_left, 0, 0);

  /*Вызов собственной функции считывания и вывода директории.
  Начало - в стартовой папке программы.*/
  dir_entry = get_dir(wnd_left, dir_entry, entries_amount_ptr, ".");
  //Бесконечный цикл работы с файлом
  while(1)
  {
    /* Любой символ считывается с клавиатуры.*/
    /*Если это не команда - он просто выводится на экран в конце*/
    ch = wgetch(wnd_left);
     /*Обновление основного окна убирает сообщения на его дне,
     относящиеся к предыдущему шагу*/
    refresh();
    switch (ch) /*Если символ командный - он будет обработан здесь*/
    {
      //Клавиши направления (стрелки)
      //Вверх
      case (char)KEY_UP:
        getyx(wnd_left, y, x);
        /*Если не край экрана - просто сдвинуться*/
        if ((y-1) >= 0)
        {
          wmove(wnd_left, y-1, x);
        }
        /*Иначе - сдвинуть сам экран*/
        else
        {
        /*Двигать только, если есть зачем - то есть не был достигнут предел
        записей в директории*/
          if (relative_y > 0)
          {
            relative_y--;
            wmove(wnd_left, 0, 1);
            wscrl(wnd_left, -1);
            /*Вывод делается здесь же.
            Возможно было бы использование отдельной функции (например, print_dir с модификацией) -
            Это бы позволило форматировать вывод нужным образом уже там, не вставляя его сюда,
            но пока что это опущено.*/
            /*Вывод не будет особым образом обрабатывать '.' и '..',
            и они будут выведенны, как есть*/
            wprintw(wnd_left, "%s", dir_entry[relative_y]->d_name);
            wmove(wnd_left, 0, 0);
          }
        }
      break;

      //Вниз
      /*Работает идентично вверху, только наоборот*/
      case (char)KEY_DOWN:
        getyx(wnd_left, y, x);
        if ((y+1) < my)
        {
          wmove(wnd_left, y+1, x);
        }
        else
        {
          if ((relative_y + 1) < *entries_amount_ptr - my)
          {
            relative_y++;
            wmove(wnd_left, my, 1);
            wscrl(wnd_left, 1);
            wprintw(wnd_left, "%s", dir_entry[my + relative_y]->d_name);
            wmove(wnd_left, my, 0);
          }
        }
      break;

      //Enter
      case 10 : /*(char)KEY_ENTER ~ 10*/
        getyx(wnd_left, y, x); /*Считать позицию нажатия*/
        /*y соответствует номеру записи в структуре*/
        /*Номер записи используется для получения и передачи имени директории*/
        strcpy(last_dir_name, dir_entry[y+relative_y]->d_name); /*Запомнить директорию*/
        dir_entry = get_dir(wnd_left, dir_entry, entries_amount_ptr, dir_entry[y+relative_y]->d_name);
        /*Чтобы после скроллинга не потерять записи выше или ниже экрана -
        вводится относительная позиция для строк*/
        /*Отн.позиция, по сути, будет указывать на то, сколько записей под "потолком" экрана
        или под его "полом", и будет модифицировать значение строки, для того,
        чтобы корректно потом получить номер записи в структуре директории*/
        if (*entries_amount_ptr > my) /*Если кол-во записей больше того, что вмещает экран*/
        {
          relative_y = *entries_amount_ptr - my; /*Записать остатки в относительную позицию*/
        }
        else
        {
          relative_y = 0;
        }
         /*Если не сменить директорию - все последующие чтения директорий
         будут идти из стартовой '.', а там нужный папок не окажется*/
        chdir(last_dir_name);
      break;

      //Выйти и программы
      case (char)KEY_F(12):
        endwin();
        for (int i = 0; i < *entries_amount_ptr; i++)
        {
          free(dir_entry[i]);
        }
        free(dir_entry);
        return 0;
      break;

      /*Ничего не произошло - значит клавиша не была обработываемой командой*/
      /*В редакторе имело смысл выводить клавишу на экран, но тут это будет мешать*/
      default:
        //wechochar(wnd_left, (char)ch);
      break;
    }
    getyx(wnd_left, y, x);
    move(my_std, 0);
    clrtoeol();
    wmove(wnd_left, y, x);
    wrefresh(wnd_left);
  }
}
