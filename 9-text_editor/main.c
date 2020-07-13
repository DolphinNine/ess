/*Программа кое-как делает самый минимум.
Большая проблема - переносы строк при сохранении и других функциях.
Нужно как-то хранить их на экране и считывать потом оттуда же*/
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 100

struct winsize size;

 /* (функция из статьи, меняющая размер окна (или скорее мешающая вылету)) */
void sig_winch()
{
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
  resizeterm(size.ws_row, size.ws_col);
  //getmaxyx(wnd, my, mx); //(нужно добавить изменение макс.размеров, при изменении окна)
}

int main() // (стоит позже переписать на получение имени)
{
  WINDOW * wnd; //Указатель на окно
  int fd; //Файловый дискриптор
  int i, j, x, y, mx, my;
  char ch; //Хранение считанного символа
  char rw_buf[BUF_SIZE]; //Буфер для чтения и записи
  void *rw_ptr = &rw_buf; //Указатель на буфер чтения

  //Открытие файла
  /*Делается до отрисовки экрана, чтобы не конфликтовать при ошибке*/
  if ((fd = open("test_file", O_RDWR)) == -1)
  {
    perror("Could not open the file!");
    return -1;
  }

  initscr();
  ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size); //Вызов функции для определения текущего размера экрана.
  signal(SIGWINCH, sig_winch); //Обработка изменения размера экрана
  curs_set(1); //Установка режима курсора
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLUE); //Пара для окна
  init_pair(2, COLOR_BLACK, COLOR_WHITE); //Пара для основоного экрана

  attron(COLOR_PAIR(2));
  bkgd(COLOR_PAIR(2));
  printw("F5 - Save | F12 - Exit");

  for (i = 1; i < size.ws_row-1; i++) //Печать номеров строк
  {
    mvprintw(i, 0, "%d", i);
  }

  //Открытие нового окна и установка его режимов
  wnd = newwin(size.ws_row-2, size.ws_col-4, 1, 2);
  keypad(wnd, TRUE); //Передавать программе коды спец.клавиш, вместо esc seq.
  wattron(wnd, COLOR_PAIR(2));
  wbkgd(wnd, COLOR_PAIR(1));
  getmaxyx(wnd, my, mx); /*Заранее запрашиваются макс.размеры окна */
  my--; /*Уменьшить на единицу для дальнейшего удобства пользования*/
  mx--;
  cbreak();
  noecho();
  refresh();
  wrefresh(wnd);
  wmove(wnd, 0, 0);

  //Вывод содержимого файла до его конца
  /*(сейчас работает на посимвольном чтении)*/
  while (read(fd, rw_ptr, 1) > 0)
  {
      wprintw(wnd, rw_buf);
  }
  wrefresh(wnd);

  //Бесконечный цикл работы с файлом
  while(1)
  {
    /* Любой символ считывается с клавиатуры.*/
    /*Если это не команда - он просто выводится на экран в конце*/
    ch = wgetch(wnd);
    switch (ch) /*Если символ командный - он будет обработан здесь*/
    {
      //Клавиши направления (стрелки)
      //Влево
      case (char)KEY_LEFT:
        getyx(wnd, y, x);
        if ((x-1) >= 0) //Проверка - не будет ли достигнута граница с этой стороны
        {
          wmove(wnd, y, x-1); /*Если всё в порядке - двигаться нормально*/
        }
        else
        {
          wmove(wnd, y-1, mx); /*Иначе - прыгнуть на строку выше*/
        }
      break;

      //Вправо
      case (char)KEY_RIGHT:
        getyx(wnd, y, x);
        if ((x+1) < mx)
        {
          wmove(wnd, y, x+1);
        }
        else
        {
          wmove(wnd, y+1, 0);
        }
      break;

      //Вверх
      case (char)KEY_UP:
        getyx(wnd, y, x);
        if ((y-1) >= 0)
        {
          wmove(wnd, y-1, x);
        }
      break;

      //Влево
      case (char)KEY_DOWN:
        getyx(wnd, y, x);
        if ((y+1) < my)
        {
          wmove(wnd, y+1, x);
        }
      break;

      //Enter
      case (char)KEY_ENTER : /*10*/
        getyx(wnd, y, x);
        mvwaddch(wnd, y, x+1, '\n'); /*На данный момент, вызов затрёт всю часть строки справа от курсора*/
        wmove(wnd, y+1, 0);
      break;

      //Стереть символ (обычное поведение backspace)
      /*за исключением того, что он будет бегать по всей строке экрана*/
      case (char)KEY_BACKSPACE:
        getyx(wnd, y, x);
        if ((x-1) < 0) /*Проверка, не упёрся ли курсор в левый край*/
        {
          /*Если упёрся - перенос на правый край строки выше*/
          wmove(wnd, y-1, mx);
          wrefresh(wnd);
          getyx(wnd, y, x);
          mvwdelch(wnd, y, x); /*Затереть символ*/
          break;
        }
        mvwdelch(wnd, y, x-1); /*Затереть символ*/
        wmove(wnd, y, x-1);
      break;

      // Запись в файл
      case (char)KEY_F(5):
        getyx(wnd, y, x); //Запомнить текущую позицию курсора
        lseek (fd, 0, SEEK_SET); //Переход в начало открытого файла
        i = 0;
        j = 0;
        for (j = 0; j < my; j++)
        {
          for (i = 0; i < mx; i++)
          {
            rw_buf[i] = mvwinch(wnd, j, i);
            // if (rw_buf[i] == '\n')
            // {
            //   break;
            // }
            /*сейчас здесь жульничество: если идёт 2 пробела подряд -
            считать это переносом строки.
            такое использование вызвано тем, что символ новой строки (newline)
            не удаётся верно прочитать*/
            if ((rw_buf[i] == ' ') && (rw_buf[i-1] == ' '))
            {
              break;
            }
          }
          rw_buf[i-1] = '\n'; //Добавить символы переноса и конца строки
          /*(программа запишет ещё и все лишние строки.
          их можно было бы отсечь, возможно, поймав все newline)*/
          if (write(fd, rw_buf, i) == -1)
          {
            perror("Could not write to the file!");
            return -1;
          }
        }
        wmove(wnd, y, x); //Вернуть курсор на позицию до начала сохранения
      break;

      //Выйти и программы
      case (char)KEY_F(12):
        endwin();
        if (close(fd) == -1)
        {
          perror("Could not close the file!");
          return -1;
        }
        return 0;
      break;

      /*Ничего не произошло - значит клавиша не была обработываемой командой*/
      /*Вывести её на экран*/
      default:
        wechochar(wnd, (char)ch);
      break;
    }
    wrefresh(wnd);
  }
}
