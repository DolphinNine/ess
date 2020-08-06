/*Основная программа сервера*/
/*Серверая программа ожидает только сообщение от клиента, автоматически
  обрабатывает его и отсылает назад, ожидая следующее.*/
/*Общение сервера с клиентом возможно и на разных узлах, а не только в пределах
  одной машины. Для этого стоит поменять адрес сервера в defs.h на реальный
  сетевой IPv4*/
/*Сервер сразу создаёт указанное в CLIENT_THREADS_AMOUNT число потоков. Эти
  потоки самостоятельно работают с клиентами через сокет - отвечают на их
  сообщения и принимают новых клиентов, если они успеют это сделать, свободны,
  и не уже загружены клиентами до максимума.*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
/*В defs.h хранятся адрес и порт соединения, используемые сервером и клиентом*/
#include "defs.h"

#define INPUT_BUF_SIZE 10
#define INBOX_BUF_SIZE 100

/*Количество клиентских потоков или просто пул*/
#define CLIENT_THREADS_POOL 3

/*Максимум массива дескрипторов для poll. По сути, равно числу обслуживаемых
  одним потоком клиентов, но -1 (0й - всегда дескриптор сокета)*/
#define POLL_DESCS_MAX 1000

void *thread_control();
void *client_thread();
void *check_input();

/*Атрибуты потоков теперь используется не только в main, что вынуждает делать
  объявление глобальным*/
pthread_attr_t threadAttr;

/*Мьютекс используемый на этапе принятия соедиения от клиента*/
pthread_mutex_t accept_lock;

/*Дескриптор сокета*/
int sock_desc;

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  const char *section = "MAIN";
  struct sockaddr_in server_addr;
  pthread_t thread_control_tid, check_input_tid;

  /*Настройка атрибутов потоков*/
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
  accept_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

  /*Установка параметров структуры адреса*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ECHO_ADDR);
  server_addr.sin_port = htons(ECHO_PORT);

/*----------------------------------------------------------------------------*/
/*Запуск сервера: подготовка сокета и запуски дополнительных функций-потоков*/

  printf("[%s] - Started\n", section);

  /*Создание сокета.*/
  printf("[%s] - Set to TCP proto\n", section);
  while ((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {}
  printf("[%s] - Socket created\n", section);

  /*Привязка сокета*/
  while ((bind(sock_desc, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1) {}
  printf("[%s] - Socket binded\n", section);

  /*TCP - перевод сокета в режим прослушки*/
  while ((listen(sock_desc, 0)) == -1) {}
  printf("[%s] - Socket set to listening mode\n", section);

  /*Запуск дополнительных функций в потоках*/
  while (pthread_create(&thread_control_tid, &threadAttr, thread_control, 0) != 0) {}
  while (pthread_create(&check_input_tid, &threadAttr, check_input, NULL) != 0) {}

/*----------------------------------------------------------------------------*/
/*Завершение работы сервера*/

  /*Собственное окончание работы одной из функций приводит к принудительному
    завершению всех оставшихся*/
  pthread_join(check_input_tid, NULL);
  pthread_cancel(thread_control_tid);

  /*Сокет закрывается*/
  while (close(sock_desc) == -1) {}
  printf("[%s] - Socket closed\n", section);

  /*Мьютекс уничтожается*/
  while (pthread_mutex_destroy(&accept_lock) != 0) {}
  printf("[%s] - Mutex destroyed\n", section);

  printf("[%s] - Shuted down\n", section);
  exit(0);
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с сокетом*/
/*Функция запускает пул резервных потоков для работы с клиентами/*/
/*Сейчас функция почти бесполезна и избыточна, так как просто запускает
  CLIENT_THREADS_POOL потоков и далее, спит и ничего не делает. От неё можно
  избавиться, вынеся её функции в main. Она, однако, всё равно оставлена,
  для реализаций функционала управления потоками в будущем: Ей легко можно было
  бы поручить отслеживание нагружености потоков. Если те не справляются - она
  будет запускать дополнительные, или наоборот, закрывать простаивающие потоки.
  Конкретной логики работы ещё не продумана.

  Варианты: Клиентский поток отчитывается о своей нагрузке, когда не может
  принять нового клиента, в глобальную переменную/очередь сообщений.
  Функция-поток контроля собирает информацию о нагрузке, и решает, достаточная
  ли она, чтобы запустить ещё поток.
  Схожим образом, клиентский поток отчитывается о простое по таймеру, если
  не имеет больше активных клиентов.*/

void *thread_control()
{
  const char *section = "THREAD_CONTROL";
  int i;
  pthread_t client_thread_tid;

  printf("[%s] - Preparing pool of %d initial threads\n", section, CLIENT_THREADS_POOL);
  for(i = 0; i < CLIENT_THREADS_POOL; i++)
  {
    while (pthread_create(&client_thread_tid, &threadAttr, client_thread, 0) != 0) {}
    printf("[%s] - Client thread [%ld] started\n", section, client_thread_tid);
  }

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    sleep(100);
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с клиентом*/
/*Вызывается из thread_control*/
/*На старте, функция блокируется в ожидании сообщения в очереди - дескриптора
  клиента. Получив его, она начинает непрерывно обрабатывать его сообщения, пока
  клиент не отключится, после чего завершается. Так как теперь используется
  очередь сообщений, контролировать процесс записи чтения дескриптора не нужно -
  только один поток клиента сможет получить дескриптор. Остальные будут ждать
  следующего шанса.*/

void *client_thread()
{
  const char *section = "C_THREAD";
  struct pollfd poll_descs[POLL_DESCS_MAX];
  char inbox[INBOX_BUF_SIZE];
  pthread_t self_tid;
  int recieved;
  int current_poll_max, i;

  /*Структура клиента*/
  int client_desc;
  struct sockaddr_in client_addr;
  socklen_t client_addr_size;
  client_addr_size = sizeof(client_addr);

  self_tid = pthread_self();
  printf("[%s #%ld] - Looking for clients in socket\n", section, self_tid);

  /*В массив poll декскрипторов заносится дескриптор сервера*/
  poll_descs[0].fd = sock_desc;
  /*Событие для проверки - есть данные для чтения*/
  poll_descs[0].events = POLLIN;
  current_poll_max = 1;

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*Проверка массива дескрипторов poll - появились ли где-то данные. Время
      ожидания -1 - вызов poll заблокируется, хоть один дескриптор не
      удовлетворит условию.*/
    poll(poll_descs, current_poll_max, -1);
    /*printf("[%s #%ld] - Some descriptor got available\n", section, self_tid);*/

    /*Если poll пройден - значит где-то сработало условие. Поиск, где именно,
      представляет из себя ручной проход по массиву декскрипторов*/
    /*Если это нулевой дескриптор - отозвался сокет сервера, а значит, можно
      попытаться принять клиента.*/
    if(poll_descs[0].revents & POLLIN)
    {
      /*Проверка, может ли текущий поток взять на себя ещё одного клиента.
        Иначе - проигноировать вызов poll.*/
      if (current_poll_max != (POLL_DESCS_MAX - 1))
      {
        /*Попытаться заблокировать зону принятия соединения от клиента. Если не
          вышло - поток продолжит работу без нового клиента*/
        if(pthread_mutex_trylock(&accept_lock) == 0)
        {
          /*Если принять соединение от клиента не получится - поток продолжит
          работу без него. Возможно, в случае ошибки здесь, клиент вовсе
          останется без обслуживания.*/
          if ((client_desc = accept(sock_desc, (struct sockaddr *)&client_addr, &client_addr_size)) != -1)
          {
            printf("[%s #%ld] - Client [%s] connected\n", section, self_tid, inet_ntoa(client_addr.sin_addr));
            current_poll_max++;
            poll_descs[current_poll_max - 1].fd = client_desc;
            poll_descs[current_poll_max - 1].events = POLLIN;
          }
          pthread_mutex_unlock(&accept_lock);
        }
      }
      /*Причина вызова обнаружена - можно уйти на следующую иттерацию, где
        поток вновь будет ожидать активности декскрипторов*/
      continue;
    }
    /*Если не сокет сервера - это дескриптор клиента. Поиск, какой именно.*/
    else
    {
      for(i = 1; i < current_poll_max; i++)
      {
        /*Сравнение события в дескрипторе с событием, которое окончило poll*/
        if(poll_descs[i].revents & POLLIN)
        {
          recieved = recv(poll_descs[i].fd, inbox, INBOX_BUF_SIZE, 0);
          if(recieved > 0)
          {
            /*Полученное сообщение модифицируется и отсылается обратно*/
            printf("[%s #%ld] - Recieved message: %s", section, self_tid, inbox);
            inbox[0] = '#';
            while ((send(poll_descs[i].fd, inbox, INBOX_BUF_SIZE, 0)) == -1) {}
            printf("[%s #%ld] - Sended redacted message back to client\n", section, self_tid);
            memset(inbox, 0, sizeof(inbox));
          }
          else
          {
            /*Если клиент закрыл сокет, это станет очевидно по нулевому числу принятых
            байт. В предыдущих версиях программы, здесь была отмена потока, но
            теперь он работает не с одним, а несколькими клиентами. Нужно
            корректно избавиться от его дексриптора и продолжить работу.*/
            if (recieved == 0)
            {
              /*Все дескрипторы ниже текущего, смещаются по массиву вверх, а
                текущее число декскрипторов уменьшается на единицу. Так
                неактивный дескриптор затирается, освобождая место в массиве*/
              for(i = i; i < current_poll_max; i++)
              {
                if (i != (POLL_DESCS_MAX - 1))
                {
                  poll_descs[i].fd = poll_descs[i + 1].fd;
                }
              }
              current_poll_max--;
              printf("[%s #%ld] - Client shuted down. Descriptor disposed.\n", section, self_tid);
            }
          }
          /*Причина вызова обнаружена - можно уйти на следующую иттерацию, где
            поток вновь будет ожидать активности декскрипторов*/
          continue;
        }
      }
    }
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток проверки ввода в терминальной строке*/
/*Вынесение работы функции в отдельный поток позволяет не блокировать работу
  всей программы сервера в ожидании ввода команды*/

void *check_input()
{
  char input[INPUT_BUF_SIZE];

  /*Бесконечный цикл. Прерывается по вводу ожидаемой команды*/
  while (1)
  {
    fgets(input, INPUT_BUF_SIZE, stdin);
    /*Команда "/shut" начинает полное завершение работы всей программы сервера*/
    if (strcmp(input, "/shut\n") == 0)
    {
      pthread_exit(NULL);
    }
  }
}
