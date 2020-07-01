#include <stdio.h>

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
