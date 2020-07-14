#include <string.h>
#include "defs.h"

int subtract(int a, int b) //Вычитание
{
	int result;
	result = a - b;
	return result;
}

//Функция передачи данных. Работа описана в add.c
int pass_name(struct list *libs, unsigned int i)
{
	strcpy(libs[i].name, "Subtraction");
	strcpy(libs[i].func_name, "subtract");
	return 0;
}
