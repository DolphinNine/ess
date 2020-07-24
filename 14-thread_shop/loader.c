/*Функция погрузчика. Заполняет случайный магазин фиксированным числом товара.
  Сам не прерывается и может быть окончен только из основной функции main*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "defs.h"

void *loader(void *shops_ptr)
{
/*---------------------------------------------------------------------------*/
/*Объявления и настройки*/

  /*Явно указывается то, что полученый указатель - указывает на стркутуру*/
  struct list *shops = shops_ptr;

  /*Хранение случайного сгенерированного номера магазина*/
  unsigned char current_shop;

  /*Генерация числа спроса текущего покупателя*/
  srand(pthread_self());
  printf("START - [LOADER] Ready to go\n");

/*---------------------------------------------------------------------------*/
/*Основная работа функции*/

  /*Бесконечный цикл*/
  while(1)
  {
    /*Случайная генерация магазина, который займёт поток погрузчика*/
    current_shop = rand() % SHOPS_AMOUNT;
    if(pthread_mutex_trylock(&shops[(int)current_shop].shop_mut) == 0)
    {
      /*Сообщить, в какой магазин зашёл погрузчик*/
      printf("[LOADER] Ocupied shop#%d (Supply: %d)(Resupplied: %d)\n", (int)current_shop + 1, shops[(int)current_shop].products, (shops[(int)current_shop].products + RESUPPLY_BASE));
      /*Число товара в магазине повышается на RESUPPLY_BASE*/
      shops[(int)current_shop].products += RESUPPLY_BASE;
      pthread_mutex_unlock(&shops[(int)current_shop].shop_mut);
    }
    sleep(1);
  }
}
