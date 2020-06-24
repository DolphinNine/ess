#include <stdio.h>

//Функции каждой из четырёх операций. Идентичны друг другу. Различаются только конечном выводе.
int add() //Сложение
{
	int a=0,b=0;
	char dump;
	printf("Input first number A: ");
	while (scanf("%d", &a) == 0) //Проверка - был ли получен ожидаемый тип (десятичный int). Если нет - сообщить об этом. Продолжать запрос, пока не будет получен корректный ввод
	{
		printf("\nMust be an integer number!\nInput first number A: ");
		scanf("%c", &dump); //Забор мусора
	}
	printf("Input second number B: ");
	while (scanf("%d", &b) == 0 )
	{
		printf("\nMust be an integer number!\nInput second number B: ");
		scanf("%c", &dump);
	}
	printf("Result is: %d\n", a+b); //Необходимая операция производится сразу в выводе. Только этот вывод и будет отличаться у разных функций
}

int subtract() //Вычитание
{
	int a=0,b=0;
	char dump;
	printf("Input first number A: ");
	while (scanf("%d", &a) == 0)
	{
		printf("\nMust be an integer number!\nInput first number A: ");
		scanf("%c", &dump);
	}
	printf("Input second number B: ");
	while (scanf("%d", &b) == 0 )
	{
		printf("\nMust be an integer number!\nInput second number B: ");
		scanf("%c", &dump);
	}
	printf("Result is: %d\n", a-b);
}

int multiply() //Умножение
{
	int a=0,b=0;
	char dump;
	printf("Input first number A: ");
	while (scanf("%d", &a) == 0)
	{
		printf("\nMust be an integer number!\nInput first number A: ");
		scanf("%c", &dump);
	}
	printf("Input second number B: ");
	while (scanf("%d", &b) == 0 )
	{
		printf("\nMust be an integer number!\nInput second number B: ");
		scanf("%c", &dump);
	}
	printf("Result is: %d\n", a*b);
}

int divide() //Деление(Целочисленное)
{
	int a=0,b=0;
	char dump;
	printf("Input first number A: ");
	while (scanf("%d", &a) == 0)
	{
		printf("\nMust be an integer number!\nInput first number A: ");
		scanf("%c", &dump);
	}
	printf("Input second number B: ");
	while (scanf("%d", &b) == 0 )
	{
		printf("\nMust be an integer number!\nInput second number B: ");
		scanf("%c", &dump);
	}
	printf("Result is: %d\n", a/b);
}

int main()
{
	char choice; //Хранение выбора главного меню
	char dump;
	//Работа главного меню и всех действий в нём происходит в бесконечном цикле	
	while(1)
	{
		printf("\n-------------------------------\n---Calc---\nChoose:\n(1) - Addition\n(2) - Subtraction\n(3) - Multiplication\n(4) - Division(Integer)\n(0) -  Quit\n-------------------------------\n");
		scanf("%c", &choice); //Считывание числа выбора в меню
		//Все варианты выбора - ветви цикла Switch. Описание выбора дополнительно дублируется после ввода числа
		switch(choice)
		{
			case '1':
			printf("(1) - Addition:\n");
			add(); //Вызов соответсвующей функции, выполняющей операцию
			break;
	
			case '2':
			printf("(2) - Subtraction:\n");
			subtract();
			break;	
		
			case '3':
			printf("(3) - Multiplication:\n");
			multiply();
			break;	
	
			case '4':
			printf("(4) - Division(Integer):\n");
			divide();
			break;
	
			case '0': //Завершение программы по выбору из меню
			printf("(0) - Quit\n");
			return 0;
			break;
			
			default: //Вариант по-умолчанию подразумевает, что успешно распознать выбор или сопоставить с доступными варинтами не удалось
			printf("\nUnrecognized option! Enter one of the options from the list!\n");
			break;
		}
		scanf("%c", &dump); //Забор всех остатоков в вводе консоли, чтобы те не попали в реальные данные
	}
}
