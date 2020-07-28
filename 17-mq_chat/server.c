/*Основная программа сервера*/
/*
  Сервер создаёт несколько очередей:
  /server_chat - очередь чата КЛИЕНТЫ->СЕРВЕР. Все клиенты пишут в эту очередь
    свои сообщения, где они читаются сервером;
  /server_service - служебная очередь КЛИЕНТЫ->СЕРВЕР. Каждый клиент, на старте
    совей работы, пишет в очередь свой pid. Этот pid используется сервером для
    создания персональной очереди клиента СЕРВЕР->КЛИЕНТ;
  /client_[pid] (например, /client_6879) - Cобственно, персональная очередь
    клиента СЕРВЕР->КЛИЕНТ. Сервер помещает в эту очередь сообщения от других
    клиентов(включая сообщения от этого же клиента). Сами клиенты постоянно
    слушают эту очередь, и выводят сообщения из неё в окне своего терминала.
  Персональный канал на каждого клиента нужен по той причине, что сообщения
  затираются в очереди, после считывания из неё. А это значит, что их мог бы
  прочитать кто-то один. Чтобы раздать это сообщение всем клиентам, оно читается
  централизованно сервером, который осуществляет пересылку этого сообщения по
  персональным очередям клиентов.

                                /<----------------<\
          /<---<(служебная_очередь)<----<\          \
        /                                 \          \
      /    />->(персональная_очередь_2)>->[КЛИЕНТ_2]  \
  [СЕРВЕР]>->(персональная_очередь_1)>->[КЛИЕНТ_1]>--->
      \                                    /
      \<------<(общая_очередь_чата)<----</

*/
/*Проблемы: Даже если клиент ушел, его очередь ещё существует. Так, однажды,
  клиенты заполнят все очереди и некуда будет помещать новых, так как очереди
  все ещё закрепленны за ушедшими. Можно удалять старые очереди, контролируя
  возвратные значения и символные значения ошибок функций работы с очередями -
  многие из них могут сообщить, что очереди уже нет. Сейчас очереди удаляет
  только сервер, но это может делать и клиент. Серверу останется это заметить и
  как-то перестроить свою память, выкинув старые pid-ы и прочие данные об
  ушедших клиентах.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>

#define SERVER_CHAT_QUEUE "/server_chat"
#define SERVER_SERVICE_QUEUE "/server_service"

#define MSG_BUF_SIZE 100
#define MSGS_AMOUNT 10

void *check_service(void *);
void *check_chat(void *);
void *check_input();

/*Структура для передачи данных функциям по указателю*/
struct list
{
  unsigned int pid[10];
  unsigned int clients_amount;
  mqd_t chat_descriptor;
  mqd_t service_descriptor;
  mqd_t client_descriptor[10];
};
/*Структура атрибутов очередей*/
struct mq_attr queueAttr;

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  struct list params;
  pthread_t check_service_tid, check_chat_tid, check_input_tid;
  char new_descriptor[20];
  int i;

  /*Настройка атрибутов потоков*/
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Настройка атрибутов очереди*/
  queueAttr.mq_flags = 0;
  queueAttr.mq_maxmsg = MSGS_AMOUNT;
  queueAttr.mq_msgsize = MSG_BUF_SIZE;
  queueAttr.mq_curmsgs = 0;

  params.clients_amount = 0;

/*----------------------------------------------------------------------------*/
/*Запуск сервера: подготовка очередей и запуски дополнительных функций-потоков*/

  printf("[SERVER] - Started\n");

  /*Создание очереди чата КЛИЕНТЫ->СЕРВЕР*/
  while ((params.chat_descriptor = mq_open(SERVER_CHAT_QUEUE, O_RDONLY | O_CREAT, 0777, &queueAttr)) == -1) {}
  printf("[SERVER] - Chat queue created\n");

  /*Создание служебной очереди КЛИЕНТЫ->СЕРВЕР*/
  while ((params.service_descriptor = mq_open(SERVER_SERVICE_QUEUE, O_RDONLY | O_CREAT, 0777, &queueAttr)) == -1) {}
  printf("[SERVER] - Service queue created\n");

  /*Запуск дополнительных функций в потоках*/
  while (pthread_create(&check_service_tid, &threadAttr, check_service, (void *)&params) != 0) {}
  while (pthread_create(&check_chat_tid, &threadAttr, check_chat, (void *)&params) != 0) {}
  while (pthread_create(&check_input_tid, &threadAttr, check_input, NULL) != 0) {}

/*----------------------------------------------------------------------------*/
/*Завершение работы сервера*/

  /*Собственное окончание работы одной из функций приводит к принудительному
    завершению всех оставшихся*/
  pthread_join(check_input_tid, NULL);
  pthread_cancel(check_service_tid);
  pthread_cancel(check_chat_tid);

  /*Очереди КЛИЕНТЫ->СЕРВЕР чата и служебная закрываются и отвязываются*/
  while (mq_close(params.chat_descriptor) == -1) {}
  while (mq_unlink(SERVER_CHAT_QUEUE) == -1) {}
  printf("[SERVER] - Chat queue closed and unlinked\n");
  while (mq_close(params.service_descriptor) == -1) {}
  while (mq_unlink(SERVER_SERVICE_QUEUE) == -1) {}
  printf("[SERVER] - Service queue closed and unlinked\n");

  /*В цилке закрываются все и отвязываются все собственные очереди
    СЕРВЕР->КЛИЕНТ*/
  for (i = 0; i < params.clients_amount; i++)
  {
    while (mq_close(params.client_descriptor[i]) == -1) {}
    sprintf(new_descriptor, "/client_%d", params.pid[i]);
    while (mq_unlink(new_descriptor) == -1) {}
    printf("[SERVER] - Client_%d queue closed and unlinked\n", params.pid[i]);
  }

  printf("[SERVER] - Shuted down\n");
  exit(0);
}

/*Функция-поток работы со служебной очередью*/
/*Слежебная очередь получает информацию о pid-ах от клиентов. Эта информация
  используется в этой же функции для создания новых потоков работы исключительно
  с новым клиентом*/
void *check_service(void *params_ptr)
{
  struct list *params = params_ptr;
  char inbox[MSG_BUF_SIZE];
  char new_descriptor[20];

  printf("[SERVICE] - Checking service queue for messages\n");

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*Ожидается сообщение в служебной очереди - pid нового клиента*/
    if(mq_receive(params->service_descriptor, inbox, MSG_BUF_SIZE, NULL) > 0)
    {
      printf("[SERVICE] - Recieved new pid: %s\n", inbox);
      /*Запомнить pid-число*/
      params->pid[params->clients_amount] = atoi(inbox);
      /*Сформировать строку-имя персональной очереди нового клиента*/
      sprintf(new_descriptor, "/client_%d", params->pid[params->clients_amount]);
      /*Создать персональную очередь клиента*/
      while ((params->client_descriptor[params->clients_amount] = mq_open(new_descriptor, O_WRONLY | O_CREAT, 0777, &queueAttr)) == -1) {}
      printf("[SERVICE] - Client_%d queue created\n", params->pid[params->clients_amount]);
      /*Общее число клиентов повышается на единицу*/
      params->clients_amount++;
    }
  }
}

/*Функция-поток работы с очередью чата*/
/*Очередь чата используется для получения сообщений от всех клиентов.
  Все клиенты знают о её существовании со старта. Функция-поток в своей работе
  пересылает сообщения с очереди чата в собственные очереди клиентов*/
void *check_chat(void *params_ptr)
{
  struct list *params = params_ptr;
  char inbox[MSG_BUF_SIZE];
  int i;

  printf("[CHAT] - Checking chat queue for messages\n");

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*Ожидается сообщение в очереди чата - простое сообщение от клиента*/
    if(mq_receive(params->chat_descriptor, inbox, MSG_BUF_SIZE, NULL) > 0)
    {
      /*Полученное сообщение пересылается всем клиентам в их персональные
        очереди.*/
      for (i = 0; i < params->clients_amount; i++)
      {
        while (mq_send(params->client_descriptor[i], inbox, strlen(inbox), 0) != 0) {}
      }
      printf("[CHAT] - Forwarded received msg from (%s) to a clients queues\n", strtok(inbox, ":"));
      memset(inbox, 0, sizeof(inbox));
    }
  }
}

/*Функция-поток проверки ввода в терминальной строке*/
/*Вынесение работы функции в отдельный поток позволяет не блокировать работу
  всей программы сервера в ожидании ввода команды*/
void *check_input()
{
  char input[MSG_BUF_SIZE];

  /*Бесконечный цикл. Прерывается по вводу ожидаемой команды*/
  while (1)
  {
    fgets(input, MSG_BUF_SIZE, stdin);
    /*Команда "/shut" начинает полное завершение работы всей программы сервера*/
    if (strcmp(input, "/shut\n") == 0)
    {
      pthread_exit(NULL);
    }
  }
}
