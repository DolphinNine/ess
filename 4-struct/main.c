#include <stdio.h>
#include <string.h>

int main()
{
	char choice; //Хранение выбора главного меню
	char s_name[5] = {'0'}; //Массив имени, используемый для временного хранения или поиска
	int s_number = 0; //Массив номера, используемый для временного хранения или поиска
	unsigned int i, comp = 0; //Переменная цикла и хранения, и переменная-флаг, используемая во время поиска записей
	
	//Базовая структора тел.книги
	struct phonebook
	{
		char name[5]; //Имя на 5 символов
		unsigned int number; //Номер телефона
	};
	
	struct phonebook friends[10]; //Книга на 10 записей
	
	//Все 10 записей сразу заполняются произвольными значениями Имя-Телефон
	strcpy(friends[0].name, "Peter");
	friends[0].number = 123456;
	
	strcpy(friends[1].name, "Ann");
	friends[1].number = 222333;
	
	strcpy(friends[2].name, "Rosie");
	friends[2].number = 456456;	
	
	strcpy(friends[3].name, "Noah");
	friends[3].number = 787878;	
	
	strcpy(friends[4].name, "Rex");
	friends[4].number = 890890;
	
	strcpy(friends[5].name, "Joe");
	friends[5].number = 900021;	
	
	strcpy(friends[6].name, "Peter");
	friends[6].number = 456321;	
		
	strcpy(friends[7].name, "Miles");
	friends[7].number = 678452;
	
	strcpy(friends[8].name, "Glenn");
	friends[8].number = 938492;	
		
	strcpy(friends[9].name, "Sade");
	friends[9].number = 984324;
	
	//Работа главного меню и всех действий в нём происходит в бесконечном цикле	
	while(1)
	{
		printf("\n-------------------------------\n---Your friends phonebook---\nChoose:\n(1) - Show all\n(2) - Search by Name\n(3) - Search by Number\n(4) - Modify existing\n(5) - Erase existing\n(0) - Quit\n-------------------------------\n");
		scanf("%c", &choice); //Считывание числа выбора в меню
		//Все варианты выбора - ветви цикла Switch. Описание выбора дополнительно дублируется после ввода числа
		switch(choice)
		{
			case '1':
			printf("(1) - Show all:\n");
			for (i = 0; i < 10; i++)
			{
				printf("%s's tel.: %d\n", friends[i].name, friends[i].number);
			}
			break;
	
			case '2':
			printf("(2) - Search by Name\n");
			printf("Enter name(up to 5 case sensitive letters): ");
			scanf("%s", s_name);
			for (i = 0; i < 10; i++)
			{
				if (strcmp(friends[i].name, s_name) == 0)
				{
					printf("\nFound entry: %s - %d\n", friends[i].name, friends[i].number);
					comp = 1;
					break;
				}
			}
			if (comp != 1)
			{
				printf("\nNo entry found\n");
			}
			break;	
		
			case '3':
			printf("(3) - Search by Number\n");
			printf("Enter number(6 digits): ");
			scanf("%d", &s_number);
			for (i = 0; i < 10; i++)
			{
				if (friends[i].number == s_number)
				{
					printf("\nFound entry: %s - %d\n", friends[i].name, friends[i].number);
					comp = 1; //Переменная помогает убедиться в конце, что совпадение было найдено.
					break;
				}
			}
			if (comp != 1) //Если нет совпадения - сообщить об этом
			{
				printf("\nNo entry found\n");
			}
			break;	
	
			case '4':
			printf("(4) - Modify existing\n");
			printf("Choose entry by number [1-10]: ");
			scanf("%d", &i);
			printf("Modifying: %s - %d\n", friends[i-1].name, friends[i-1].number);
			printf("Enter new name(up to 5 letters): ");
			scanf("%s", s_name);
			printf("Enter new number(up to 6 digits): ");
			scanf("%d", &s_number);
			strcpy(friends[i-1].name, s_name);
			friends[i-1].number = s_number;
			printf("New entry: %s - %d\n", friends[i-1].name, friends[i-1].number);
			break;
	
			case '5':
			printf("(5) - Erase existing\n");
			printf("Choose entry by number [1-10]: ");
			scanf("%d", &s_number);
			printf("Erasing: %s - %d\n", friends[s_number-1].name, friends[s_number-1].number);
			//Затирание записи - это её зануление
			for (i = 0; i < 5; i++)
			{
				friends[s_number-1].name[i] = '0';
			}
			friends[s_number-1].number = 0;
			break;
	
			case '0': //Завершение программы по выбору из меню
			printf("(0) - Quit\n");
			return 0;
			break;
			
			default: //Вариант по-умолчанию подразумевает, что успешно распознать выбор или сопоставить с доступными варинтами не удалось
			printf("\nUnrecognized option! Enter one of the options from the list!\n");
			break;
		}
		scanf("%c"); //Забор всех остатоков в вводе консоли, чтобы те не попали в реальные данные
		comp = 0; //Зануление переменной, указывающей на успешное/неуспешное сравнение при поиски записей
	}
}
