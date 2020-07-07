#include <stdio.h>

int take_var(char variable)
{
  int value;
  char dump;
  printf("Input variable %c: ", variable);
  while (scanf("%d", &value) == 0) //Проверка - был ли получен ожидаемый тип (десятичный int). Если нет - сообщить об этом. Продолжать запрос, пока не будет получен корректный ввод
  {
    printf("\nMust be an integer number!\n"\
    "Input variable %c: ", variable);
    scanf("%c", &dump); //Забор мусора
  }
  return value;
}
