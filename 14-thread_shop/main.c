/*Основная функция, заполнения магазинов и запуска потоков покупателей и
  погрузчика, а также окончания работы последнего*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
/*defs.h хранит все определённые константы. Некоторые из них - общие для
  разных функций*/
#include "defs.h"
#include "proto.h"

int main()
{
/*---------------------------------------------------------------------------*/
/*Объявления и настройки*/

  /*Массив тидов покупателей*/
  pthread_t customer_tid[CUSTOMERS_AMOUNT];
  /*Один тид погрузчика*/
  pthread_t loader_tid;

  /*Объявление структуры данных о магазинах*/
  struct list shops[SHOPS_AMOUNT];

  int i;

  /*Настройка атрибутов потоков*/
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /*Настройка мьютексов*/
  for(i = 0; i < SHOPS_AMOUNT; i++)
  {
    shops[i].shop_mut = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  }

  /*Зерно для генератора чисел берётся на основании времени*/
  srand(time(NULL));

/*---------------------------------------------------------------------------*/
/*Основная работа функции и запуски*/

  /*Генерация запасов товаров магазинов*/
  printf("------Setting up shops------\n");
  for(i = 0; i < SHOPS_AMOUNT; i++)
  {
    /*К базовому числу товаров PRODUCT_BASE прибавляется случайно
      сгенерированное отклонение от -PRODUCT_VARIANCE до +PRODUCT_VARIANCE*/
    shops[i].products = PRODUCT_BASE + (rand() % (PRODUCT_VARIANCE*2 + 1) - PRODUCT_VARIANCE);
    printf("[SHOP#%d] Opened with %d products\n", i + 1, shops[i].products);
  }

  /*Запуск потоков покупателей*/
  printf("\n------Launching customer and loader threads------\n");
  for(i = 0; i < CUSTOMERS_AMOUNT; i++)
  {
    pthread_create(&customer_tid[i], &threadAttr, customer, (void *)&shops);
  }
  /*Запуск потока погрузчика*/
  pthread_create(&loader_tid, &threadAttr, loader, (void *)&shops);

  /*Ожидание конца работы потоков покупателей*/
  for(i = 0; i < CUSTOMERS_AMOUNT; i++)
  {
    pthread_join(customer_tid[i], NULL);
  }
  /*Завершение работы погрузчика*/
  pthread_cancel(loader_tid);

  /*Выходная проверка запасов товаров магазинов*/
  printf("\n------Closing up shops------\n");
  for(i = 0; i < SHOPS_AMOUNT; i++)
  {
    printf("[SHOP#%d] Closed with %d products\n", i + 1, shops[i].products);
  }
  exit(0);
}
