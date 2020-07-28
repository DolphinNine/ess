/*Основная программа клиента*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>

#define SERVER_CHAT_QUEUE "/server_chat"
#define SERVER_SERVICE_QUEUE "/server_service"

#define MSG_BUF_SIZE 100

void *check_self_queue(void *);

/*Основная функция клиента. Подготовка данных и запуски функций-потоков*/
int main()
{
  mqd_t chat_descriptor, service_descriptor, client_descriptor;
  pid_t pid;
  pthread_t check_self_queue_tid;
  char input[MSG_BUF_SIZE - 10];
  char message[MSG_BUF_SIZE + 2];
  char nickname[10];
  char self_descriptor[20];
  char *char_position;

  /*Настройка атрибутов потоков*/
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

/*----------------------------------------------------------------------------*/
/*Запуск клиента: подготовки очередей и запуски дополнительных функций-потоков*/

  printf("[CLIENT] - Started\n");

  /*Открытие очередей чата и служебной очереди КЛИЕНТЫ->СЕРВЕР. Они уже должны
    быть созданы сервером. Если нет - клиент заблокируется на открытии
    соответствующей очереди, в ожидании её создания*/
  while ((chat_descriptor = mq_open(SERVER_CHAT_QUEUE, O_WRONLY)) == -1) {}
  printf("[CLIENT] - Chat queue opened\n");
  while ((service_descriptor = mq_open(SERVER_SERVICE_QUEUE, O_WRONLY)) == -1) {}
  printf("[CLIENT] - Service queue opened\n");

  /*Клиент получает собственный pid и оправляет его серверу в служебной очереди*/
  pid = getpid();
  sprintf(message, "%d", pid);
  while (mq_send(service_descriptor, message, strlen(message), 0) != 0) {}
  /*Больше клиент работать со служебной очередью не будет, и её можно закрыть*/
  while (mq_close(service_descriptor) == -1) {}
  printf("[CLIENT] - Service queue closed\n");

  /*Для более понятного человеку взаимодействия, клиент потребует ввести прозвище*/
  printf("Enter your nickname: ");
  fgets(nickname, 10, stdin);
  char_position = strchr(nickname, '\n');
  *char_position = '\0';

  /*Вероятно, к этому моменту сервер уже создаст персональную очередь для
    клиента. Так как она создаётся сервером, опираясь на pid клиента, её
    имя можно восстановить на любой из сторон из этого же pid-а.
    Очередь открываетя клиентом для чтения. Если её ещё нет - работа блокируется
    в ожидании.*/
  sprintf(self_descriptor, "/client_%d", pid);
  while ((client_descriptor = mq_open(self_descriptor, O_RDONLY)) == -1) {}
  printf("[CLIENT] - Clients own queue opened\n");

  /*Запуск дополнительной функции-потока*/
  while (pthread_create(&check_self_queue_tid, &threadAttr, check_self_queue, (void *)&client_descriptor) != 0) {}

/*----------------------------------------------------------------------------*/
/*Работа чата*/

  printf("--------CHAT--------\n");
  /*Бесконечный цикл. Прерывается только по вводу определённой команды*/
  while (1)
  {
    fgets(input, (MSG_BUF_SIZE - 10), stdin);
    /*Если введена нужная команда, то вместо пересылки сообщения, работа цикла
      завершается*/
    if (strcmp(input, "/shut\n") == 0)
    {
      break;
    }
    /*Пока это не команда - это простое сообщение. Оно пересылается в общую
      очередь чата КЛИЕНТЫ->СЕРВЕР*/
    else
    {
      /*В терминал отправляется строка со служебными символами.
        Используются управляющие последовательности(escape sequences):
        \033[A - перенос курсора на строку выше
        \33[2K - затереть текущую строку
        \r - "carriage return" перенести курсор на начальную позицию линии.
        Ввод строки такого формата приводит к тому, что на стороне клиента, после
        ввода сообщения, строка с этим сообщением тут же затирается. Клиент,
        однако всё равно увидит своё сообщение, но полученное уже от сервера,
        а не в собственном вводе.
        Красивее было бы делать интерфейс на базе ncurses, но это требует больше
        времени.*/
      printf("\033[A\33[2K\r");
      /*Клиент сам дополняет сообщение своим прозвищем и нужным форматированием*/
      sprintf(message, "%s: %s", nickname, input);
      while (mq_send(chat_descriptor, message, strlen(message), 0) != 0) {}
      memset(message, 0, sizeof(message));
      memset(input, 0, sizeof(input));
    }
  }

/*----------------------------------------------------------------------------*/
/*Завершение работы клиента*/

  /*Клиенту достаточно закрыть очереди, с которыми он работал, и которые ещё
    не были закрыты*/
  while (mq_close(chat_descriptor) == -1) {}
  printf("[CLIENT] - Chat queue closed\n");
  while (mq_close(client_descriptor) == -1) {}
  /*while (mq_unlink(self_descriptor) == -1) {}*/
  printf("[CLIENT] - Clients own queue closed\n");

  printf("[CLIENT] - Shuted down\n");
  exit(0);
}

void *check_self_queue(void *param)
{
  mqd_t client_descriptor = *((mqd_t *)param);
  char inbox[MSG_BUF_SIZE];

  while(1)
  {
    if(mq_receive(client_descriptor, inbox, MSG_BUF_SIZE, NULL) > 0)
    {
      printf("%s", inbox);
      memset(inbox, 0, sizeof(inbox));
    }
  }
}
