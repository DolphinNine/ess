#include <string.h>
#include "defs.h"

int add(int a, int b) //Сложение
{
	int result;
	result = a + b;
	return result;
}

//Функция передачи данных об основной функции этого файла (в данном случае add)
int pass_name(struct list *libs, unsigned int i) //Получает указатель на структуру и номер выделенной там записи
{
	strcpy(libs[i].name, "Addition"); //Отдать имя для пользователя
	strcpy(libs[i].func_name, "add"); //Отдать имя функции
	return 0;
}
