/*Значения, используемые main*/
#define PRODUCT_BASE 500
#define PRODUCT_VARIANCE 50
#define CUSTOMERS_AMOUNT 3

/*Значения, используемые customer*/
#define DEMAND_BASE 10000
#define DEMAND_VARIANCE 200

/*Значения, используемые loader*/
#define RESUPPLY_BASE 500

/*Общие значения*/
#define SHOPS_AMOUNT 5

/*Структура данных о магазинах. Создаётся структурой для возможности передачи
  функциям по void указателю*/
struct list
{
  /*Массив магазинов. Хранит число товаров*/
  unsigned int products;
  /*Массив мьютексов магазинов*/
  pthread_mutex_t shop_mut;
};
