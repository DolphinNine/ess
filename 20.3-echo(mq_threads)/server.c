/*Основная программа сервера*/
/*Серверая программа ожидает только сообщение от клиента, автоматически
  обрабатывает его и отсылает назад, ожидая следующее.*/
/*Общение сервера с клиентом возможно и на разных узлах, а не только в пределах
  одной машины. Для этого стоит поменять адрес сервера в defs.h на реальный
  сетевой IPv4*/
/*Сервер сразу создаёт указанное в CLIENT_THREADS_AMOUNT число резервных потоков
  (пул) работы с клиентами. Все эти потоки ожидают данных о клиенте в очереди
  сообщений. Первый, кто успеет считать из очереди данные, начнёт обслуживать
  клиента, а остальные - ждать следующей возможности.*/

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
#include <mqueue.h>
/*В defs.h хранятся адрес и порт соединения, используемые сервером и клиентом*/
#include "defs.h"

#define INPUT_BUF_SIZE 10
#define INBOX_BUF_SIZE 100

/*Количество резервных потоков или просто пул*/
#define CLIENT_THREADS_POOL 3

#define CLIENT_DESC_QUEUE "/desc_queue"

void *socket_control(void *);
void *client_thread();
void *check_input();

/*Атрибуты потоков теперь используется не только в main, что вынуждает делать
  объявление глобальным*/
pthread_attr_t threadAttr;

mqd_t client_desc_queue;

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  const char *section = "MAIN";
  struct sockaddr_in server_addr;
  pthread_t socket_control_tid, check_input_tid;
  int sock_desc;

  /*Структура атрибутов очередей*/
  struct mq_attr queueAttr;

  /*Настройка атрибутов потоков*/
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Установка параметров структуры адреса*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ECHO_ADDR);
  server_addr.sin_port = htons(ECHO_PORT);

  /*Настройка атрибутов очереди*/
  queueAttr.mq_flags = 0;
  queueAttr.mq_maxmsg = 10;
  queueAttr.mq_msgsize = 100;
  queueAttr.mq_curmsgs = 0;

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

  /*Создание очереди*/
  while ((client_desc_queue = mq_open(CLIENT_DESC_QUEUE, O_RDWR | O_CREAT, 0777, &queueAttr)) == -1) {}
  printf("[%s] - Clients descriptors MQ created\n", section);

  /*Запуск дополнительных функций в потоках*/
  while (pthread_create(&socket_control_tid, &threadAttr, socket_control, (void *)&sock_desc) != 0) {}
  while (pthread_create(&check_input_tid, &threadAttr, check_input, NULL) != 0) {}

/*----------------------------------------------------------------------------*/
/*Завершение работы сервера*/

  /*Собственное окончание работы одной из функций приводит к принудительному
    завершению всех оставшихся*/
  pthread_join(check_input_tid, NULL);
  pthread_cancel(socket_control_tid);

  /*Сокет закрывается*/
  while (close(sock_desc) == -1) {}
  printf("[%s] - Socket closed\n", section);

  while (mq_close(client_desc_queue) == -1) {}
  while (mq_unlink(CLIENT_DESC_QUEUE) == -1) {}
  printf("[%s] - MQ closed and unlinked\n", section);

  printf("[%s] - Shuted down\n", section);
  exit(0);
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с сокетом*/
/*Функция запускает пул резервных потоков для работы с клиентами, который
  пополняется, как только все потоки станут заняты. Функция ожидает подключения
  от клиента, приняв которое, она отсылает дескриптор клиента в очередь
  сообщений и ожидает следующего подключения.*/

void *socket_control(void *param_ptr)
{
  const char *section = "SOCKET_CONTROL";
  int *sock_desc = param_ptr;
  int i;
  pthread_t client_thread_tid;
  char accepted_desc[5];

  /*Вместо целой структуры, её элементы объявляются теперь по отдельности.*/
  int client_desc;
  struct sockaddr_in client_addr;
  socklen_t client_addr_size;

  client_addr_size = sizeof(client_addr);
  i = CLIENT_THREADS_POOL;

  printf("[%s] - Waiting for clients to connect\n", section);

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*Если уже достигнут предел резервных потоков - запустить ещё один пул.
    Это условие достигается при первом входе в цикл, а также каждый раз, когда
    весь пул резервных потоков оказался занят клиентами.*/
    if(i == CLIENT_THREADS_POOL)
    {
      printf("[%s] - Preparing pool of %d reserve threads\n", section, CLIENT_THREADS_POOL);
      for(i = 0; i < CLIENT_THREADS_POOL; i++)
      {
        while (pthread_create(&client_thread_tid, &threadAttr, client_thread, 0) != 0) {}
        printf("[%s] - Client thread [%ld] started\n", section, client_thread_tid);
      }
      i = 0;
    }

    /*TCP - ожидание подключения клиента*/
    while ((client_desc = accept(*sock_desc, (struct sockaddr *)&client_addr, &client_addr_size)) == -1) {}
    printf("[%s] - Client [%s] connected\n", section, inet_ntoa(client_addr.sin_addr));

    sprintf(accepted_desc, "%d", client_desc);
    while (mq_send(client_desc_queue, accepted_desc, strlen(accepted_desc), 0) != 0) {}
    printf("[%s] - Client descriptor [%d] sended to MQ\n", section, client_desc);

    i++;
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с клиентом*/
/*Вызывается из socket_control*/
/*На старте, функция блокируется в ожидании сообщения в очереди - дескриптора
  клиента. Получив его, она начинает непрерывно обрабатывать его сообщения, пока
  клиент не отключится, после чего завершается. Так как теперь используется
  очередь сообщений, контролировать процесс записи чтения дескриптора не нужно -
  только один поток клиента сможет получить дескриптор. Остальные будут ждать
  следующего шанса.*/

void *client_thread()
{
  const char *section = "C_THREAD";
  char inbox[INBOX_BUF_SIZE];
  pthread_t self_tid;
  int recieved;
  int client_desc;

  self_tid = pthread_self();
  printf("[%s #%ld] - Looking for clients descriptors in MQ\n", section, self_tid);

  /*Ожидание сообщения в очереди. Если поток успеет проверить первым - он
    заберёт дескриптор клиента. Иначе, будет ждать дальше.*/
  while (mq_receive(client_desc_queue, inbox, INBOX_BUF_SIZE, NULL) <= 0) {}
  client_desc = atoi(inbox);
  printf("[%s #%ld] - Took accepted client from MQ\n", section, self_tid);

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*TCP - Ожидается сообщение из сокета*/
    recieved = recv(client_desc, inbox, INBOX_BUF_SIZE, 0);
    if(recieved > 0)
    {
      /*Полученное сообщение модифицируется и отсылается обратно*/
      printf("[%s #%ld] - Recieved message: %s", section, self_tid, inbox);
      inbox[0] = '#';
      while ((send(client_desc, inbox, INBOX_BUF_SIZE, 0)) == -1) {}
      printf("[%s #%ld] - Sended redacted message back to client\n", section, self_tid);
      memset(inbox, 0, sizeof(inbox));
    }
    else
    {
      /*Если клиент закрыл сокет, это станет очевидно по нулевому числу принятых
      байт. В таком случае, поток отменяет сам себя*/
      if (recieved == 0)
      {
        printf("[%s #%ld] - Client shuted down. Closing thread.\n", section, self_tid);
        pthread_cancel(self_tid);
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
