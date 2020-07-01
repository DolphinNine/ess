#include <stdio.h>

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
