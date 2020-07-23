/*Рутина работы с задачами копирования, вставки и отрисовки окон
  состояния этих процессов.*/
/*(проблемы: плохая отрисовка в слишком маленьких окнах; не самое надёжное
  центрование текста в подокне)*/
#include <curses.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "proto.h"

/*Структура для передачи данных функциям по указателю*/
struct list
{
  WINDOW *wnd; /*Активное окно*/
  unsigned int total_size; /*Общий размер файла*/
  unsigned int written_size; /*Объём уже записанных в копию данных*/
  char cp_file_name[255]; /*Имя файла*/
  char cp_file_cwd[PATH_MAX]; /*Путь файла-оригинала*/
};

void *paste_file(void *);
void *draw_progress_win(void *);

/*Основная функция работы с потоками. Получает извне указатель на окно, а также
  имя и путь до копируемого файла*/
int cp_rountine(WINDOW *wnd, char cp_file_name[255], char cp_file_cwd[PATH_MAX])
{
  void *return_value_paste, *return_value_draw;
  int y, x;
  struct list params;
  struct stat file_info;
  pthread_t paste_func_tid, draw_func_tid;

  /*Настройка атрибутов потоков*/
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Получение данных о файле и их занесение в массив для обмена с потоками*/
  /*(имеет смысл перенести данные о файле из main или из main сюда, так как
    в случае успешного копирования, stat вызывается дважды для одно и того же
    файла)*/
  stat(cp_file_cwd, &file_info);
  params.total_size = file_info.st_size;
  params.written_size = 0; /*Объём записаного на старте - ноль*/
  params.wnd = wnd;
  strcpy(params.cp_file_name, cp_file_name);
  strcpy(params.cp_file_cwd, cp_file_cwd);

  /*Вызов функции копирования файла и рисовки окна в потоках*/
  if(pthread_create(&paste_func_tid, &threadAttr, paste_file, (void *)&params) != 0)
  {
    print_message(params.wnd, -1, "CP ROUTINE: Paste thread creation failed");
    return -1;
  }
  if (pthread_create(&draw_func_tid, &threadAttr, draw_progress_win, (void *)&params) != 0)
  {
    print_message(params.wnd, -1, "CP ROUTINE: Progress win thread creation failed");
    return -1;
  }
  /*Ожидание завершения работы потоков*/
  pthread_join(paste_func_tid, &return_value_paste);
  pthread_join(draw_func_tid, &return_value_draw);
  if ((int *)return_value_paste > 0 || (int *)return_value_paste > 0)
  {
    return -1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
/*Функция - поток непосредственно переноса файла. Полагается на данные,
  подготовленные основной cp_routine и без них не сработает*/
/*---------------------------------------------------------------------------*/

void *paste_file(void *params_ptr)
{
  /*Подготовка данных*/
  struct list *params = params_ptr;
  int fd_original, fd_copy;
  char write_buf[4096];
  ssize_t read_amount, written_amount;

/*---------------------------------------------------------------------------*/
/*Открытие дескрипторов файла-оригинала и копии*/

  /*Оригинал - режим: только чтение*/
  if ((fd_original = open(params->cp_file_cwd, O_RDONLY)) < 0)
  {
    print_message(params->wnd, -1, "PASTE THREAD: Original file descriptor opening failed");
    return (void *)-1;
  }
  /*Копия - режим: только запись; создать файл при открытии; не перезаписывать,
    если файл уже существует; конкретизация прав доступа созданного файла*/
  if ((fd_copy = open(params->cp_file_name, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
  {
    /*(немного расширив чтение ошибки, можно было бы даже сообщить, что именно
      с файлом не так: существует ли он, или ещё что-то)*/
    print_message(params->wnd, -1, "PASTE THREAD: Copy file descriptor opening failed");
    return (void *)-1;
  }

/*---------------------------------------------------------------------------*/
/*Чтение из оригина и запись в копию*/

  /*Пока есть что читать...*/
  while ((read_amount = read(fd_original, write_buf, sizeof(write_buf))) > 0)
  {
    /*Пока что-то пишется...*/
    while ((written_amount = write(fd_copy, write_buf, read_amount)) > 0)
    {
      /*Добавить к общему записанному объёму текущий. Эти данные ожидает
        поток отрисовки прогресса*/
      params->written_size += written_amount;
      read_amount -= written_amount;
    }
  }
  /*Нечего писать и нечего читать - закрыть файлы и окончить работу*/
  close(fd_copy);
  close(fd_original);
  return 0;
}

/*---------------------------------------------------------------------------*/
/*Функция - поток отслеживания прогресса записи. Полагается на данные,
  подготовленные основной cp_routine а также постоянно считывает прогресс записи
  из паралельного потока.*/
/*---------------------------------------------------------------------------*/

void *draw_progress_win(void *params_ptr)
{
  /*Подготовка данных*/
  float percents, scale;
  int my, mx, i, x;
  struct list *params = params_ptr;
  WINDOW *progress_subwin;

  /*Отрытие нового подокна(subwin)*/
  getmaxyx(params->wnd, my, mx);
  /*derwin аналогичен subwin, но координаты будут относительно родительского
    окна, а не общего окна*/
  progress_subwin = derwin(params->wnd, 5, mx/2, my/3, mx/4);
  wattron(progress_subwin, COLOR_PAIR(2));
  wbkgd(progress_subwin, COLOR_PAIR(2));
  werase(progress_subwin);

  getmaxyx(progress_subwin, my, mx);
  mvwprintw(progress_subwin, 0, mx/3, "Copying...");
  wrefresh(progress_subwin);
  /*Печать пустых мест шкалы прогресса*/
  for (i = 3; i < mx-3; i++)
  {
    mvwaddch(progress_subwin, 2, i, (char)95); /*'_' - 95*/
  }
  wrefresh(progress_subwin);
  /*Работа с подсчётом и отрисовкой прогресса*/
  i = 1; /*Итерация. Участвует в определении числа закрашенных позиций шкалы*/
  x = 3; /*Координата текущей закрашенной позиции шкалы*/
  do
  {
    /*Подсчёт процентов*/
    percents = (((float)params->written_size / params->total_size) * 100);
    /*Подсчет условного этапа прогресса заполнения*/
    scale = (((float)100 / (mx - 3)) * i);
    /*Печать записаного / общего объёма данных в килобайтах*/
    mvwprintw(progress_subwin, 1, mx/4, "%d Kbytes / %d Kbytes (%.0f %)", params->written_size/1024, params->total_size/1024, percents);
    /*Если текущий процент больше или равен, чем ближайший этап прогресса...*/
    /*Как это работает:
    *
    *  Например, общая длинна шкалы - 50 символов.
    *  Значит, каждые 100 / 50 = 2 процента должно происходить заполнение одного
    *  символа. Это и будет одним этапом.
    *  Заполнение шкалы будет происходить каждый раз, когда:
    *  ПРОЦЕНТ >= ЭТАП * ИТЕРАЦИЯ
    *  Первое заполнение произодёт, когда: ПРОЦЕНТ >= (2 * 1) = 2.
    *  Второе: ПРОЦЕНТ >= (2 * 2) = 4, и так далее.
    */
    if(percents >= scale)
    {
      /*...заполнить ещё одну позицию шкалы прогресса*/
      mvwaddch(progress_subwin, 2, x, (char)219); /*'█' - 219*/
      i++;
      /*Не выводить символов больше, чем общая длинна шкалы прогресса*/
      /*(иначе печатается буквально на один символ больше)*/
      if(x < (mx - 4))
      {
        x++;
      }
    }
    wrefresh(progress_subwin);
    /*Проходить по циклу, пока объём записанного не поровняется с общим*/
  } while (params->written_size != params->total_size);

  /*Известить об окончании*/
  mvwprintw(progress_subwin, 4, mx/3, "Done! Press any key.");
  wrefresh(progress_subwin);
  /*Дождаться нажатия любой клавиши и окончить работу*/
  getch();
  delwin(progress_subwin);
  refresh();
  return 0;
}
