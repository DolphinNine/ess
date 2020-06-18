#include <stdio.h>

int main()
{
    unsigned int d = 0xDDCCBBAA;
    char A = 'A';
    char B = 'B';
    char *ptr1;
    char **ptr2;
    
    //Вывод байтов числа в цикле
    ptr1 = &d;
    for (int i=0;i<4;i++)
    {
        printf("byte #%d: 0x%x\n", i+1, *ptr1&0xFF); //Маска 0xFF добавлена для исключения вывода типа 0xFFFFFFAA (откуда берутся единицы?)
        ptr1++;
    }
    printf("\n");
    
    //Изменение третьего байта числа (CC -> 25)
    ptr1 = &d;
    ptr1+=2;//Скачок на 2 размера типа (char) - переход на третий байт
    *ptr1 = 0x25;
    printf("changed byte #3 to 0x25: 0x%x\n", d);
    printf("\n");
    
    //Изменение указателя через двойной указатель
    ptr1 = &A;
    ptr2 = &ptr1;
    printf("ptr1 leads to: %c\n",*ptr1);
    **ptr2 = 'B';
    printf("ptr1 leads to: %c\n",*ptr1);
}
