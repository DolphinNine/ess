/*Порт соединения. Используется и сервером, и клиентом*/
#define ECHO_PORT 1234
/*Сетевой IPv4 адресс сервера. Используется и сервером, и клиентом*/
/*Это должен быть сетевой адрес сервера. Главное условие - он должен быть
  единым для сервера и клиента.*/
#define ECHO_ADDR "127.0.0.1" /*127.0.0.1 - собственный локальный*/

/*Мьютекс используемый в работе со структурой клиента*/
pthread_mutex_t client_struct_lock;

/*Структура данных о клиенте. Сейчас потоки пользуются только полем дискриптора,
  возвращаемым accept. Остальные поля просто не используются, но могут
  понадобиться в будущем.*/
struct list
{
  int client_desc;
  struct sockaddr_in client_addr;
  socklen_t client_addr_size;
};
