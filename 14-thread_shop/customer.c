/*Функция покупателя. В случайном порядке посещает магазины и собирает
  оттда товары, пока не удовлетворит свой спрос*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "defs.h"

void *customer(void *shops_ptr) //unsigned int *shop, pthread_mutex_t *shop_mut
{
/*---------------------------------------------------------------------------*/
/*Объявления и настройки*/

  /*Явно указывается то, что полученый указатель - указывает на стркутуру*/
  struct list *shops = shops_ptr;

  /*Хранение числа спроса текущего покупателя. Объявлено знаковым, чтобы
    можно было определить, насколько перенасытился покупатель и вернуть
    это число назад в магазин.*/
  int demand;
  /*Хранение случайного сгенерированного номера магазина, который занял поток
    покупателя. Это будет число, просто оно достаточно небольше, чтобы хранить
    его в однобайтной переменной*/
  unsigned char current_shop;
  char name_letter;

  /*Генерация числа спроса текущего покупателя*/
  /*Здесь для зерна используется собственный тид потока. Если использовать, как
    и в main, время, то придётся вводить паузу между созданием потоков, иначе
    они создаются так быстро, что получат одно и то же зерно, а это приведёт
    к генерации одних и тех же чисел*/
  srand(pthread_self());
  demand = DEMAND_BASE + (rand() % (DEMAND_VARIANCE*2 + 1) - DEMAND_VARIANCE);
  /*Временное решение проблемы наименования - покупатель генерирует для себя
    букву-имя и использует её, для обозначения себя в потоке. Не спасает
    от вероятности сгененировать уже занятую букву, но пока в программе нет
    блокированого/передаваемого значения или имени для покупателя - будет
    использоваться такой подход.*/
  name_letter = 'A' + (random() % 26);
  printf("START - [CUSTOMER %c] Demand set for %d\n", name_letter, demand);
  //printf("START - [CUSTOMER %ld] Demand set for %d\n", pthread_self(), demand);

/*---------------------------------------------------------------------------*/
/*Основная работа функции*/

  /*Цикл, прерываемый только по насыщению покупателя*/
  while(demand != 0)
  {
    /*Случайная генерация магазина, который займёт поток покупателя*/
    current_shop = rand() % SHOPS_AMOUNT;
    if(pthread_mutex_trylock(&shops[(int)current_shop].shop_mut) == 0)
    {
      /*Сообщить, в какой магазин зашёл покупатель*/
      printf("[CUSTOMER %c] Ocupied shop#%d (Demand: %d)(Supply: %d)\n", name_letter, (int)current_shop + 1, demand, shops[(int)current_shop].products);
      /*Спрос становится меньше на число товара в магазине*/
      demand -= shops[(int)current_shop].products;
      /*Сам магазин также теряет это число, то есть списывается вообще весь товар
      магазина*/
      shops[(int)current_shop].products -= shops[(int)current_shop].products;
      /*Однозначно известно сейчас, что в магазине 0 товара. Однако неизвестно,
      удовлетворило ли это спрос покупателя. Это и проверяется далее*/
      /*Спрос не удовлетворён*/
      if(demand > 0)
      {
        /*Сказать, что покупатель ушёл в спячку на текущем магазине, и сообщить
        сколько спроса ещё осталось*/
        printf("[CUSTOMER %c] Fell asleep on shop#%d (Demand: %d)(Supply: %d)\n", name_letter, (int)current_shop + 1, demand, shops[(int)current_shop].products);
      }
      /*Покупатель перенасытился (спрос стал отрицательным)*/
      if(demand <= 0)
      {
        /*Вернуть в магазин лишний товар*/
        shops[(int)current_shop].products += -(demand);
        /*Обнулить спрос*/
        demand += -(demand);
        /*Сообщить, где спрос был удовлетворён*/
        printf("STOP - [CUSTOMER %c] Satisfied on shop#%d (Supply: %d)\n", name_letter, (int)current_shop + 1, shops[(int)current_shop].products);
        /*Разорвать цикл*/
      }
      pthread_mutex_unlock(&shops[(int)current_shop].shop_mut);
    }
    sleep(2);
  }
  return 0;
}
