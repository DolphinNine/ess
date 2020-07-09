#include <string.h>
#include "defs.h"

int multiply(int a, int b) //Умножение
{
	int result;
	result = a * b;
	return result;
}

//Функция передачи данных. Работа описана в add.c
char pass_name(struct list *libs, unsigned int i)
{
	strcpy(libs[i].name, "Multiplication");
	strcpy(libs[i].func_name, "multiply");
	return 0;
}
