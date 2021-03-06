#include <stdio.h>
#define N 9 //Размер массива. Программа должна работать с любыми числами, но массивы квадрат размера которых даёт трёзначные числа (от 10 и более) не выглядят "красиво" при выводе, так как форматирование расчитано на двухзначные числа максимум.

//Функция вывода массива
int print_array(int A[N][N])
{
    for (int j=0;j<N;j++)
    {
        for (int i=0;i<N;i++)
        {
            //Вывод таким образом, чтобы однозначные и двузначные числа были на одном уровне "таблицы"
            if (A[i][j]/10!=0) 
            {
            	printf("%d ",A[i][j]);
            }
            else 
            {
            	printf("%d  ",A[i][j]); //Добавляется лишний знак - пробел
            }
        }
        printf("\n");
    }
    printf("\n");	
}

int main()
{
    int A[N][N];
    int b=0,c=0,l=0,i,j,x,y;
    
    //Заполнение массива от 1 до N^2
    printf("---1 to N^2 array---\n");
    for (j=0;j<N;j++)
    {
        for (i=0;i<N;i++)
        {
            A[i][j]=b+1;
            b++;
        }
    }
    print_array(A); //Вывод массива через функцию
    
//---------------------------------------------------------------------------------  
    //Вывод массива в обратном порядке. Повторяет обычный вывод из фунции, но инвертирован
    printf("---Reversed array---\n");
    for (j=N-1;j>=0;j--)
    {
        for (i=N-1;i>=0;i--)
        {
            if (A[i][j]/10!=0) 
            {
            	printf("%d ",A[i][j]);
            }
            else 
            {
            	printf("%d  ",A[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
    
//---------------------------------------------------------------------------------    
    //Заполнение массива треугольниками 0-1
    printf("---0-1 array---\n");
    for (j=0;j<N;j++)
    {
        for (i=0;i<N;i++)
        {
            //Единицы в массиве могут располагается на определнном числе мест. Это число начинается с последнего элемента в первой строке и растёт с каждой новой строкой на 1.
            if (i>=N-1-l)
            {
            	A[i][j]=1;
            }
            else 
            {
            	A[i][j]=0;
            }
        }
        l++; //Увеличение колличества единиц для следующей строки на 1
    }
    print_array(A);
    
//---------------------------------------------------------------------------------      
    //Заполнение массива по спирали
    printf("---Spiral array---\n");
    //Индексы элементов массива
    i=0;
    j=0;
    
    l=0; //Переменная, ограничивающая длинну ребра квадрата. Вначале длинна не ограничена(0). Также используется для определения нуля координат текущего квадрата.
    for (b=1;b<=N*N;b++) //Цикл идёт по числу, заполняющему массив
    {
    	if ((i==l)&(j==l)) //Если в верхнем левом углу - идти вправо
    	{
		x=1; //Направление движения по строке (i)
    		y=0; //Направление движения по столбцу (j)
    		c++; //Счётчик углов. Растёт после каждого такого угла.
    	} 
    	if ((i==N-1-l)&(j==l)) //Если в верхнем правом углу - идти вниз
    	{
    		x=0;
    		y=1;
    		c++;
    	} 
    	if ((i==N-1-l)&(j==N-1-l)) //Если в нижнем правом углу - идти влево
    	{
    		x=-1;
    		y=0;
    		c++;
    	} 
    	if ((i==l)&(j==N-1-l)) //Если в нижнем левом углу - идти вверх
    	{
    		x=0;
    		y=-1;
    		c++;
    	}
    	if (c>4) //Если пройдены все 4 угла, значит внешний периметр пройден, и можно перейти к тому, что находится ниже, относительно внешнего
    	{
    		c=1; //Счётчк углов не обнуляется, а сбрасывается до одного. Это вызвано потерей любого первого угла меньших квадратов, кроме самого первого (внешнего), так как на шаге с ним будет осуществленно движение, которое не вызовет роста счётчка углов (с).
    		l++; //Длинна ребра текущего квадрата сокращается
    		
    		//Новый координатный ноль - верхний левый угол нового, сокращённого квадрата
    		i=l; 
    		j=l;
    	} 
    	A[i][j]=b; //Элемент массива получает значение
    	
    	//Направление движения, определённое ранее, применяется к текущей координате. Так осуществляется движение по массиву
    	i+=x; 
    	j+=y;
    }
    print_array(A);
}
