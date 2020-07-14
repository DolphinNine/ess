#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "proto.h"
#include "defs.h"

int main()
{
	int choice; //Хранение выбора главного меню
	char dump;
	unsigned int *ls_size_ptr; //Указатель на размер списка библиотек
	int a, b;
	unsigned int ls_size = 1;

	int (*curr_func)(int a, int b); //Указатель на текущую используемую функцию, полученную из dlsym

	ls_size_ptr = &ls_size;
	libs = libs_init(ls_size_ptr);

	//Работа главного меню и всех действий в нём происходит в бесконечном цикле
	while(1)
	{
		printf("\n-------------------------------\n"\
		"---Calc---\n"\
		"Choose:\n");
		//Вывод доступных опций в цикле. Имена функций берутся из структуры, в которую, до этого, сами функции и записали данные о себе
		for (int i = 0; i < ls_size; i++)
		{
			printf("(%d) - %s\n", i+1, libs[i].name);
		}
		printf("(0) - Quit\n"\
		"-------------------------------\n");
		//Считывание числа выбора в меню
		while(scanf("%d", &choice) == 0) //Было получено НЕ число
		{
			printf("Unrecognized option! Enter one of the options from the list!\n");
			scanf("%c", &dump);
		}
		if(choice > ls_size) //Число выбора выходит за пределы доступных опций
		{
			printf("Must be a number from the list!\n");
			choice = -1;
			continue; //Продолжить цикл while с самого начала, пропуская последующие шаги
		}
		if (choice == 0) //Выбран выход из программы
		{
			printf("(0) - Quit\n");
			//Закрытие библиотек
			for (int i = 0; i < ls_size; i++)
			{
				dlclose(libs[i].handle); //Закрыть каждую библиотеку
			}
			free(libs); //Высвободить память, отведённую под стркутуру
			return 0;
		}
		//Теперь выбор(choice) влияет на вызываемую функцию напрямую. Ранее, здесь был switch-case цикл
		printf("(%d) - %s\n", choice, libs[choice-1].name); //Продублировать имя выбраной функции
		a = take_var('A');
		b = take_var('B');
		curr_func = dlsym(libs[choice-1].handle, libs[choice-1].func_name); //Вызов нужной функции по указателю, полученному от dlopen ранее
		printf ("Result: %d", (*curr_func)(a, b)); //Вызов соответсвующей функции, выполняющей операцию
		scanf("%c", &dump); //Забор всех остатоков в вводе консоли, чтобы те не попали в реальные данные
	}
}
