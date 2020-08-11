/*Основная программа клиента*/
/*Клиентская программа работает с RAW сокетом через UDP. Работающий с ней сервер
 - это тот же сервер, что был в изначальном "echo" проекте, который
 компилируется в режиме работы с UDP.*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
/*В defs.h хранятся адрес и порт соединения, используемые сервером и клиентом*/
#include "defs.h"

/*Максимальный размер пакета в целом(64k)*/
#define PACKET_SIZE 65535
/*Размер данных в пакете = PACKET_SIZE - 8(заголовок)*/
#define PAYLOAD_SIZE 65527
/*Собственный порт клиента*/
#define SELF_PORT 4321

void *check_socket();
void *check_input();

/*Структура сервера объявляется глобальной, так как используется по всех
  функциях*/
struct sockaddr_in server_addr;
/*Дескриптор сокета*/
int sock_desc;

/*----------------------------------------------------------------------------*/
/*Основная функция сервера. Подготовка данных и запуски функций-потоков*/

int main()
{
  const char *section = "MAIN";

  /*Параметры для setsockopt.*/
  const int optval = 1;
  const socklen_t optlen = 1;

  pthread_t check_socket_tid, check_input_tid;

  /*Настройка атрибутов потоков*/
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Установка параметров структуры адреса*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ECHO_ADDR);
  server_addr.sin_port = htons(ECHO_PORT);


/*----------------------------------------------------------------------------*/
/*Запуск сервера: подготовка сокета и запуски дополнительных функций-потоков*/

  printf("[%s] - Started\n", section);

  while ((sock_desc = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1) {}
  printf("[%s] - Socket created\n", section);

  while ((setsockopt(sock_desc, IPPROTO_IP, IP_HDRINCL, &optval, optlen)) == -1) {}
  printf("[%s] - Socket options set\n", section);

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
  printf("[%s] - Socket closed\n", section);

  printf("[%s] - Shuted down\n", section);
  exit(0);
}

/*----------------------------------------------------------------------------*/
/*Функция-поток работы с сокетом*/
/*Функция находится в бесконечном режиме ожидания сообщений из сокета*/

void *check_socket()
{
  const char *section = "SOCKET";
  struct sockaddr_in source_addr;
  socklen_t source_addr_size = sizeof(struct sockaddr_in);

  /*Буфер всего пакета*/
  char in_packet[PACKET_SIZE];
  /*Указатель на данные в пакете*/
  char *in_payload;

  /*Указатель на данные принятого пакета работает схожим с отправленым пакетом
    образом, но он указывает на позицию чуть дальше, так как в ответном
    сообщении приходится учитывать ещё и IP заголовок, а не только UDP.*/
  in_payload = (char *)(in_packet + sizeof(struct udphdr) + sizeof(struct iphdr));

  printf("[%s] - Checking socket for messages\n", section);

  /*Бесконечный цикл. Функция может быть окончена только принудительно из main*/
  while(1)
  {
    /*Ожидается сообщение из сокета. Клиент получит все сообщения из сокета,
      включая своё собственное и некоторый мусор, который попадёт в сокет. Его
      можно и отсечь, например, добавив проверку порта, если клиентский порт
      отличается от серверного.*/
    if(recvfrom(sock_desc, in_packet, PACKET_SIZE, 0, (struct sockaddr *)&source_addr, &source_addr_size) > 0)
    {
      printf("[%s] - Recieved message from [%s]: %s", section, inet_ntoa(source_addr.sin_addr), in_payload);
      memset(in_packet, 0, PACKET_SIZE);
    }
  }
}

/*----------------------------------------------------------------------------*/
/*Функция-поток проверки ввода в терминальной строке*/
/*Вынесение работы функции в отдельный поток позволяет не блокировать работу
  всей программы сервера в ожидании ввода команды*/
/*Клиентская реализация фунции отличается от серверной тем, что если считанная
  строка не является командой для программы, она считается сообщением, которое
  отправляется в сокет.*/

void *check_input()
{
  const char *section = "INPUT";

  /*Структура заголовка пакета*/
  struct udphdr *udp_header;
  struct iphdr *ip_header;

  char out_packet[PACKET_SIZE];
  char input[PAYLOAD_SIZE];
  char *out_payload;

  socklen_t server_addr_size = sizeof(server_addr);

  /*Заполнение заголовков*/
  /*IP-заголовок. Идёт самым первым*/
  ip_header = (struct iphdr *)out_packet;
  /*UDP-заголовок. Идёт после IP-заголовка*/
  udp_header = (struct udphdr *)(out_packet + sizeof(struct iphdr));
  /*Сами данные - идут после всех заголовков*/
  out_payload = (char *)(out_packet + sizeof(struct iphdr) + sizeof(struct udphdr));

  /*Заполнение структур заголовков.*/
  /*IP*/
  /*Параметры, которые должны быть заполнены автоматически*/
  ip_header->check = 0;
  ip_header->saddr = 0;
  ip_header->id = 0;
  ip_header->tot_len = 0;
  /*Параметры, заполняемые вручную*/
  ip_header->ihl = 5;
  ip_header->version = 4;
  ip_header->tos = 16;
  ip_header->ttl = 64;
  ip_header->protocol = 17;
  ip_header->daddr = inet_addr(ECHO_ADDR);
  /*UDP*/
  /*Порт клиента и сервера отличаются - так избегаются вечные пересылки клиента
    самому себе, при работе на локальной машине*/
  udp_header->source = htons(SELF_PORT);
  udp_header->dest = htons(ECHO_PORT);
  udp_header->check = htons(0);


  printf("[%s] - Ready to send message to the socket\n", section);

  /*Бесконечный цикл. Прерывается по вводу ожидаемой команды*/
  while (1)
  {
    fgets(input, PAYLOAD_SIZE, stdin);
    /*Команда "/shut" начинает полное завершение работы всей программы сервера*/
    if (strcmp(input, "/shut\n") == 0)
    {
      pthread_cancel(pthread_self());
    }
    /*Если это не собственная команда программы - это сообщение, которе нужно
    отправить в сокет.*/
    else
    {
      /*Ввод переносится в пакет, в заголовок заносится его разме + размер
      ввода, а после, весь пакет целиком отправляется в сокет*/
      strcpy(out_payload, input);
      udp_header->len = htons(sizeof(struct udphdr) + strlen(out_payload));

      while ((sendto(sock_desc, out_packet, (sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(out_payload)), 0, (struct sockaddr *)&server_addr, server_addr_size)) == -1) {}
      printf("[%s] - Sended message to the socket\n", section);

      memset(out_payload, 0, PAYLOAD_SIZE);
    }
  }
}
