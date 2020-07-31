/*Основная программа клиента*/
/*Клиентская программа создаёт сокет, при TCP устанавливает подключение, после
  чего ожидает ввода от пользователя. Если это не команда завершения работы
  клиентского приложения, то ввод будет отправлен по сокету серверу.
  Параллельно, клиент ожидает сообщений из сокета. Это должно быть ответное
  сообщение от сервера. В режиме UDP клиент получит любое сообщение*/

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

#define INPUT_BUF_SIZE 100
#define INBOX_BUF_SIZE 100

void *check_socket(void *);
void *check_input();

/*Структура сервера объявляется глобальной, так как используется по всех
  функциях*/
struct sockaddr_in server_addr;

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
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

  printf("[CLIENT] - Started\n");

  /*Создание сокета. Используется условная компиляция меняющая рабочий протокол*/
  #ifdef TCP
  #undef NO_MODE
  printf("[CLIENT] - Set to TCP proto\n");
  while ((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {}
  #endif

  #ifdef UDP
  #undef NO_MODE
  printf("[CLIENT] - Set to UDP proto\n");
  while ((sock_desc = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {}
  #endif

  /*Если протокол не был установлен - выход из программы (описана в defs.h)*/
  #ifdef NO_MODE
  printf("[CLIENT] - Protocol was not set!\nShutting down now\n");
  exit(0);
  #endif

  printf("[CLIENT] - Socket created\n");

  /*TCP - Подключение к серверу. Программа блокируется в этом этапе до подключения*/
  #ifdef TCP
  while ((connect(sock_desc, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1) {}
  printf("[CLIENT] - Connected to the [%s] server\n", inet_ntoa(server_addr.sin_addr));
  #endif

  /*Запуск дополнительных функций в потоках*/
  while (pthread_create(&check_socket_tid, &threadAttr, check_socket, (void *)&sock_desc) != 0) {}
  while (pthread_create(&check_input_tid, &threadAttr, check_input, (void *)&sock_desc) != 0) {}

/*----------------------------------------------------------------------------*/
/*Завершение работы клиента*/

  /*Собственное окончание работы одной из функций приводит к принудительному
    завершению всех оставшихся*/
  pthread_join(check_input_tid, NULL);
  pthread_cancel(check_socket_tid);

  /*Сокет закрывается*/
  while (close(sock_desc) == -1) {}
  printf("[CLIENT] - Socket closed\n");

  printf("[CLIENT] - Shuted down\n");
  exit(0);
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с сокетом*/
/*Функция находится в бесконечном режиме ожидания сообщений из сокета*/

void *check_socket(void *param_ptr)
{
  int *sock_desc = param_ptr;
  char inbox[INBOX_BUF_SIZE];

  /*UDP - функции будут использовать стрктуру адреса отправителя. Сама она
    уже объявлена, но требуется ещё и размер этой структуры*/
  #ifdef UDP
  socklen_t server_addr_size = sizeof(server_addr);
  #endif

  printf("[SOCKET] - Checking socket for messages\n");

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*TCP - Ожидается сообщение из сокета*/
    #ifdef TCP
    if(recv(*sock_desc, inbox, INBOX_BUF_SIZE, 0) > 0)
    {
      /*Полученное сообщение выводится в терминал*/
      printf("[SOCKET] - Recieved message: %s", inbox);
      memset(inbox, 0, sizeof(inbox));
    }
    #endif

    /*UDP - Ожидается сообщение из сокета. Работа идентична TCP, но использует
      другие функции и данные*/
    #ifdef UDP
    if(recvfrom(*sock_desc, inbox, INBOX_BUF_SIZE, 0, (struct sockaddr *)&server_addr, &server_addr_size) > 0)
    {
      printf("[SOCKET] - Recieved message from [%s]: %s", inet_ntoa(server_addr.sin_addr), inbox);
      memset(inbox, 0, sizeof(inbox));
    }
    #endif
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток проверки ввода в терминальной строке*/
/*Вынесение работы функции в отдельный поток позволяет не блокировать работу
  всей программы сервера в ожидании ввода команды*/
/*Клиентская реализация фунции отличается от серверной тем, что если считанная
  строка не является командой для программы, она считается сообщением, которое
  отправляется в сокет.*/

void *check_input(void *param_ptr)
{
  char input[INPUT_BUF_SIZE];
  int *sock_desc = param_ptr;

  /*UDP - функции будут использовать стрктуру адреса отправителя. Сама она
    уже объявлена, но требуется ещё и размер этой структуры*/
  #ifdef UDP
  socklen_t server_addr_size = sizeof(server_addr);
  #endif

  printf("[SOCKET] - Ready to send message to the socket\n");

  /*Бесконечный цикл. Прерывается по вводу ожидаемой команды*/
  while (1)
  {
    fgets(input, INPUT_BUF_SIZE, stdin);
    /*Команда "/shut" начинает полное завершение работы всей программы сервера*/
    if (strcmp(input, "/shut\n") == 0)
    {
      pthread_exit(NULL);
    }
    /*Если это не собственная команда программы - это сообщение, которе нужно
      отправить в сокет.*/
    else
    {
      #ifdef TCP
      while ((send(*sock_desc, input, INPUT_BUF_SIZE, 0)) == -1) {}
      #endif

      #ifdef UDP
      while ((sendto(*sock_desc, input, INPUT_BUF_SIZE, 0, (struct sockaddr *)&server_addr, server_addr_size)) == -1) {}
      #endif
      printf("[CLIENT] - Sended message to the socket\n");
    }
  }
}
