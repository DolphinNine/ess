/*Основная программа сервера*/
/*Серверая программа ожидает только сообщение от клиента, автоматически
  обрабатывает его и отсылает назад, ожидая следующее.*/
/*Общение сервера с клиентом возможно и на разных узлах, а не только в пределах
  одной машины. Для этого стоит поменять адрес сервера в defs.h на реальный
  сетевой IPv4*/
/*TCP-сервер работает только с одним клиентом, с которым первым будет
  установлена связь.
  UDP-сервер работает со всеми клиентами, работающими с этим же сокетом
  Это не проблема, просто, в логике программы, при TCP сервер ждёт лишь одного
  клиента*/

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
void *check_input();

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  struct sockaddr_in server_addr;
  pthread_t check_socket_tid, check_input_tid;
  int sock_desc;

  /*Настройка атрибутов потоков*/
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Установка параметров структуры адреса*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ECHO_ADDR);
  server_addr.sin_port = htons(ECHO_PORT);

/*----------------------------------------------------------------------------*/
/*Запуск сервера: подготовка очередей и запуски дополнительных функций-потоков*/

  printf("[SERVER] - Started\n");

  /*Создание сокета. Используется условная компиляция меняющая рабочий протокол*/
  #ifdef TCP
  #undef NO_MODE
  printf("[SERVER] - Set to TCP proto\n");
  while ((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {}
  #endif

  #ifdef UDP
  #undef NO_MODE
  printf("[SERVER] - Set to UDP proto\n");
  while ((sock_desc = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {}
  #endif

  /*Если протокол не был установлен - выход из программы (описана в defs.h)*/
  #ifdef NO_MODE
  printf("[SERVER] - Protocol was not set!\nShutting down now\n");
  exit(0);
  #endif

  printf("[SERVER] - Socket created\n");

  /*Привязка сокета*/
  while ((bind(sock_desc, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1) {}
  printf("[SERVER] - Socket binded\n");

  /*TCP - перевод сокета в режим прослушки*/
  #ifdef TCP
  while ((listen(sock_desc, 0)) == -1) {}
  printf("[SERVER] - Socket set to listening mode\n");
  #endif

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
/*При TCP, функция блокируется в ожидании подключения от клиента. Минуя этот
  этап, функция переходит в бесконечный режим ожидания сообщения из сокета, его
  модификации и отправки назад отправителю*/

void *check_socket(void *param_ptr)
{
  int *sock_desc = param_ptr;
  #ifdef TCP
  int client_desc;
  #endif
  char inbox[INBOX_BUF_SIZE];
  struct sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(client_addr);

  printf("[SOCKET] - Waiting for client to connect\n");

  /*TCP - ожидание подключения клиента*/
  #ifdef TCP
  while ((client_desc = accept(*sock_desc, (struct sockaddr *)&client_addr, &client_addr_size)) == -1) {}
  printf("[SOCKET] - Client [%s] connected\n", inet_ntoa(client_addr.sin_addr));
  #endif

  printf("[SOCKET] - Checking socket for messages\n");

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*TCP - Ожидается сообщение из сокета*/
    #ifdef TCP
    if(recv(client_desc, inbox, INBOX_BUF_SIZE, 0) > 0)
    {
      /*Полученное сообщение модифицируется и отсылается обратно*/
      printf("[SOCKET] - Recieved message: %s", inbox);
      inbox[0] = '#';
      while ((send(client_desc, inbox, INBOX_BUF_SIZE, 0)) == -1) {}
      printf("[SOCKET] - Sended redacted message back to client\n");
      memset(inbox, 0, sizeof(inbox));
    }
    #endif

    /*UDP - Ожидается сообщение из сокета. Работа идентична TCP, но использует
      другие функции и данные*/
    #ifdef UDP
    if(recvfrom(*sock_desc, inbox, INBOX_BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_size) > 0)
    {
      /*Отличие от TCP-сервеа - при UDP сервер получает любые сообщения. Также,
        происходит заполнение структуры адреса, чем можно воспользоваться,
        выведя адрес отправителя.*/
      printf("[SOCKET] - Recieved message from [%s]: %s", inet_ntoa(client_addr.sin_addr), inbox);
      inbox[0] = '#';
      while ((sendto(*sock_desc, inbox, INBOX_BUF_SIZE, 0, (struct sockaddr *)&client_addr, client_addr_size)) == -1) {}
      printf("[SOCKET] - Sended redacted message back to sender\n");
      memset(inbox, 0, sizeof(inbox));
    }
    #endif
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
