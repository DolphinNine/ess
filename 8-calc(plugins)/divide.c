#include <string.h>
#include "defs.h"

int divide(int a, int b) //Деление(Целочисленное)
{
	int result;
	result = a / b;
	return result;
}

//Функция передачи данных. Работа описана в add.c
int pass_name(struct list *libs, unsigned int i)
{
	strcpy(libs[i].name, "Division(Integer)");
	strcpy(libs[i].func_name, "divide");
	return 0;
}
