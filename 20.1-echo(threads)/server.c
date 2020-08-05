/*Основная программа сервера*/
/*Серверая программа ожидает только сообщение от клиента, автоматически
  обрабатывает его и отсылает назад, ожидая следующее.*/
/*Общение сервера с клиентом возможно и на разных узлах, а не только в пределах
  одной машины. Для этого стоит поменять адрес сервера в defs.h на реальный
  сетевой IPv4*/
/*В отличие от предыдущей версии TCP сервера, текущая, в теории, не ограниченна
  по числу клиентов и может работать с ними параллельно. При подключении нового
  клиента, для него создаётся отдельный поток обработки его сообщений.
  Когда клиент завершает работу (вызов close() сокета на стороне клиента),
  выделенный ему поток самостоятельно закрывается.*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*В defs.h хранятся адрес и порт соединения, используемые сервером и клиентом*/
#include "defs.h"

#define INPUT_BUF_SIZE 10
#define INBOX_BUF_SIZE 100

void *check_socket(void *);
void *client_thread(void *);
void *check_input();

/*Атрибуты потоков теперь используется не только в main, что вынуждает делать
  объявление глобальным*/
pthread_attr_t threadAttr;

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  struct sockaddr_in server_addr;
  pthread_t check_socket_tid, check_input_tid;
  int sock_desc;

  /*Настройка атрибутов потоков*/
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
  /*Инициализация мьютекса*/
  client_struct_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

  /*Установка параметров структуры адреса*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ECHO_ADDR);
  server_addr.sin_port = htons(ECHO_PORT);

/*----------------------------------------------------------------------------*/
/*Запуск сервера: подготовка сокета и запуски дополнительных функций-потоков*/

  printf("[SERVER] - Started\n");

  /*Создание сокета.*/
  printf("[SERVER] - Set to TCP proto\n");
  while ((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {}
  printf("[SERVER] - Socket created\n");

  /*Привязка сокета*/
  while ((bind(sock_desc, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1) {}
  printf("[SERVER] - Socket binded\n");

  /*TCP - перевод сокета в режим прослушки*/
  while ((listen(sock_desc, 0)) == -1) {}
  printf("[SERVER] - Socket set to listening mode\n");

  /*Запуск дополнительных функций в потоках*/
  while (pthread_create(&check_socket_tid, &threadAttr, check_socket, (void *)&sock_desc) != 0) {}
  while (pthread_create(&check_input_tid, &threadAttr, check_input, NULL) != 0) {}

/*----------------------------------------------------------------------------*/
/*Завершение работы сервера*/

  /*Собственное окончание работы одной из функций приводит к принудительному
    завершению всех оставшихся*/
  pthread_join(check_input_tid, NULL);
  pthread_cancel(check_socket_tid);

  /*Сокет закрывается*/
  while (close(sock_desc) == -1) {}
  printf("[SERVER] - Socket closed\n");

  printf("[SERVER] - Shuted down\n");
  exit(0);
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с сокетом*/
/*Функция всё ещё блокируется в ожидании подключения от клиента, однако теперь,
  приняв подключение, она запускает отдельный поток работы только с этим
  клиентом. После этого, функция ожидает подключения следующего клиента.*/

void *check_socket(void *param_ptr)
{
  int *sock_desc = param_ptr;
  pthread_t client_thread_tid;

  /*clietn_struct объявлена в defs.h*/
  struct list client_struct, client_struct_local;
  client_struct.client_addr_size = sizeof(client_struct.client_addr);

  printf("[SOCKET] - Waiting for clients to connect\n");

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*TCP - ожидание подключения клиента*/
    while ((client_struct_local.client_desc = accept(*sock_desc, (struct sockaddr *)&client_struct_local.client_addr, &client_struct_local.client_addr_size)) == -1) {}

    /*Глобальная структура хранения данных о клиенте (client_struct) - всего
    одна, однако даннные в неё по очереди пишутся для разных потоков.
    Чтобы избежать потери данных из-за ресурсной гонки, структура блокируется
    на записи данных, и на их копировании в потоках работы с клиентами.
    Но это не спасёт от ситуации, в которой поток клиента не успеет считать
    данные до их перезаписи. (ввести паузу/дополнительные хранилища данных?)*/
    pthread_mutex_lock(&client_struct_lock);
    memcpy(&client_struct, &client_struct_local, sizeof(client_struct));
    pthread_mutex_unlock(&client_struct_lock);

    printf("[SOCKET] - Client [%s] connected\n", inet_ntoa(client_struct.client_addr.sin_addr));

    while (pthread_create(&client_thread_tid, &threadAttr, client_thread, (void *)&client_struct) != 0) {}
    printf("[SOCKET] - Client thread [%ld] started\n", client_thread_tid);
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с клиентом*/
/*Вызывается из check_socket, получая от той указатель на структуру с данными
  о клиенте.*/
/*Функция непрерывно ожидает сообщения от клиента, обрабаывает их и отсылает
  назад. Если клиент закрыл сокет на своей стороне, функция обнаруживает это
  и вызывает отмену (pthread_cancel) собственного потока.*/

void *client_thread(void *params_ptr)
{
  struct list client_struct;
  char inbox[INBOX_BUF_SIZE];
  pthread_t self_tid;
  int recieved;

  /*Данные с адреса копируются в локальную копию структуры клиента. Это
  позволяет не хранить весь массив данных о клиентах в глобальном массиве
  структуры, но создаёт проблемы с ресурсной гонкой*/
  pthread_mutex_lock(&client_struct_lock);
  memcpy(&client_struct, params_ptr, sizeof(client_struct));
  pthread_mutex_unlock(&client_struct_lock);

  self_tid = pthread_self();
  printf("[C_THREAD #%ld] - Recieving clients messages\n", self_tid);

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*TCP - Ожидается сообщение из сокета*/
    recieved = recv(client_struct.client_desc, inbox, INBOX_BUF_SIZE, 0);
    if(recieved > 0)
    {
      /*Полученное сообщение модифицируется и отсылается обратно*/
      printf("[C_THREAD #%ld] - Recieved message: %s", self_tid, inbox);
      inbox[0] = '#';
      while ((send(client_struct.client_desc, inbox, INBOX_BUF_SIZE, 0)) == -1) {}
      printf("[C_THREAD #%ld] - Sended redacted message back to client\n", self_tid);
      memset(inbox, 0, sizeof(inbox));
    }
    else
    {
      /*Если клиент закрыл сокет, это станет очевидно по нулевому числу принятых
      байт. В таком случае, поток отменяет сам себя*/
      if (recieved == 0)
      {
        printf("[C_THREAD #%ld] - Client shuted down. Closing thread.\n", self_tid);
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
