#include <stdio.h>

int main()
{
    unsigned int a = 0xDDCCBBAA;
    unsigned int b;
    //вывод байтов числа в цикле
    for (int i=0;i<4;i++)
    {
        b = ((a >> i*8) & 0xFF);
        printf("byte #%d: 0x%x\n", i+1, b);
    }
    //изменение третьего байта числа (CC -> 25)
    b = a ^ 0xE90000;
    printf("changed byte #3 to 0x25: %x\n", b);
}
