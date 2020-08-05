/*Основная программа сервера*/
/*Серверая программа ожидает только сообщение от клиента, автоматически
  обрабатывает его и отсылает назад, ожидая следующее.*/
/*Общение сервера с клиентом возможно и на разных узлах, а не только в пределах
  одной машины. Для этого стоит поменять адрес сервера в defs.h на реальный
  сетевой IPv4*/
/*Сервер сразу создаёт указанное в CLIENT_THREADS_AMOUNT число резервных потоков
  (пул) работы с клиентами. Все эти потоки почти сразу блокируются через
  семафоры, которые разблокируются только из управляющего потока(check_socket).
  Приняв соединение от клиента, управляющий поток разблокирует семафор одного
  из клиентских потоков, который уже самостоятельно забирает данные клиента и
  работает независимо.*/

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
#include <semaphore.h>
/*В defs.h хранятся адрес и порт соединения, используемые сервером и клиентом*/
#include "defs.h"

#define INPUT_BUF_SIZE 10
#define INBOX_BUF_SIZE 100

/*Количество резервных потоков или просто пул*/
#define CLIENT_THREADS_POOL 3

void *check_socket(void *);
void *client_thread(void *);
void *check_input();

/*Атрибуты потоков теперь используется не только в main, что вынуждает делать
  объявление глобальным*/
pthread_attr_t threadAttr;

/*Объявление структуры клиента теперь тоже вынесенно в глобальную секцию. Это
  можно было сделать и раньше, но необходимость появиалсь только в текущей
  реализации.*/
struct list client_struct;

/*Глабальное объявление семафора позволяет уничтожить их по окончанию работы из
  главной функции*/
sem_t client_struct_lock[CLIENT_THREADS_POOL];

/*Структура данных о клиенте. Сейчас потоки пользуются только полем дискриптора,
  возвращаемым accept. Остальные поля просто не используются, но могут
  понадобиться в будущем.*/
struct list
{
  int client_desc;
  struct sockaddr_in client_addr;
  socklen_t client_addr_size;
};

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  struct sockaddr_in server_addr;
  pthread_t check_socket_tid, check_input_tid;
  int sock_desc;
  int i;

  /*Настройка атрибутов потоков*/
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Установка параметров структуры адреса*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ECHO_ADDR);
  server_addr.sin_port = htons(ECHO_PORT);
  client_struct.client_addr_size = sizeof(client_struct.client_addr);

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

  /*Уничтожение семафоров*/
  for(i = 0; i < CLIENT_THREADS_POOL; i++)
  {
    sem_destroy(&client_struct_lock[i]);
  }
  printf("[SERVER] - Semaphores destroyed\n");

  printf("[SERVER] - Shuted down\n");
  exit(0);
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с сокетом*/
/*Функция запускает пул резервных потоков для работы с клиентами, но блокирует
  им возможность считать данные о них. Получив соединение от клиента, функция
  разблокирует один из потоков.*/

void *check_socket(void *param_ptr)
{
  int *sock_desc = param_ptr;
  int i;
  pthread_t client_thread_tid;

  /*client_struct объявлена глобально*/
  struct list client_struct_local;
  client_struct_local.client_addr_size = sizeof(client_struct_local.client_addr);

  /*Инициализация семафоров. Их значение уже на старте равно нулю.*/
  for(i = 0; i < CLIENT_THREADS_POOL; i++)
  {
    sem_init(&client_struct_lock[i], 0, 0);
  }

  printf("[SOCKET] - Waiting for clients to connect\n");

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*Если уже достигнут предел резервных потоков - запустить ещё один пул.
    Это условие достигается при первом входе в цикл, а также каждый раз, когда
    весь пул резервных потоков оказался занят клиентами.*/
    if(i == CLIENT_THREADS_POOL)
    {
      printf("[SOCKET] - Preparing pool of %d reserve threads\n", CLIENT_THREADS_POOL);
      for(i = 0; i < CLIENT_THREADS_POOL; i++)
      {
        /*Поток получает только указатель на семафор.*/
        while (pthread_create(&client_thread_tid, &threadAttr, client_thread, (void *)&client_struct_lock[i]) != 0) {}
        printf("[SOCKET] - Client thread [%ld] started\n", client_thread_tid);
      }
      i = 0;
    }

    /*TCP - ожидание подключения клиента*/
    while ((client_struct_local.client_desc = accept(*sock_desc, (struct sockaddr *)&client_struct_local.client_addr, &client_struct_local.client_addr_size)) == -1) {}
    printf("[SOCKET] - Client [%s] connected\n", inet_ntoa(client_struct_local.client_addr.sin_addr));

    /*Клиент был подключен. Данные о нём помещаются в глобальную стркутуру...*/
    memcpy(&client_struct, &client_struct_local, sizeof(client_struct));
    /*...а семафор незанятого потока инкрементируется*/
    sem_post(&client_struct_lock[i]);
    i++;
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с клиентом*/
/*Вызывается из check_socket, получая от той указатель на семафор*/
/*На старте, функция блокируется в ожидании инкремента семафора от управлющего
  потока. После этого, функция забирает данные и непрерывно ожидает сообщения от
  клиента, обрабатывает их и отсылает назад. Если клиент закрыл сокет на своей
  стороне, функция обнаруживает это и вызывает отмену (pthread_cancel)
  собственного потока.*/

void *client_thread(void *lock_ptr)
{
  struct list client_struct_local;
  char inbox[INBOX_BUF_SIZE];
  pthread_t self_tid;
  int recieved;

  /*Пока нет клиента, поток блокирется здесь. Разблокировка возможна только из
  управлющего потока (check_socket)*/
  sem_wait((sem_t *)lock_ptr);
  /*Как только клиент был присоединён, семафор разблокируется, и поток может
  забрать данные о клиенте. Далее, ни семафор, ни глобальная структура клиента
  участвовать в этом потоке не будут.*/
  memcpy(&client_struct_local, &client_struct, sizeof(client_struct_local));

  self_tid = pthread_self();
  printf("[C_THREAD #%ld] - Recieving clients messages\n", self_tid);

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*TCP - Ожидается сообщение из сокета*/
    recieved = recv(client_struct_local.client_desc, inbox, INBOX_BUF_SIZE, 0);
    if(recieved > 0)
    {
      /*Полученное сообщение модифицируется и отсылается обратно*/
      printf("[C_THREAD #%ld] - Recieved message: %s", self_tid, inbox);
      inbox[0] = '#';
      while ((send(client_struct_local.client_desc, inbox, INBOX_BUF_SIZE, 0)) == -1) {}
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
