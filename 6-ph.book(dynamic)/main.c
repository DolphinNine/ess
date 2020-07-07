//Основная функция меню, вызова команд и вывода результатов
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "proto.h"
#include "defs.h"

int main()
{
	char choice; //Хранение выбора главного меню
	char s_name[5] = {'0'}; //Массив имени, используемый для временного хранения или поиска
	unsigned int s_number = 0; //Массив номера, используемый для временного хранения или поиска
	int i; //Переменная цикла и хранения
	int pb_size = 10; //Размер телефонной книги. На старте равен 10
	char dump; //Переменная для сброса муссора вывода в конце цикла
	char *pb_size_ptr;

	//Книга на 10 начальных записей
	struct phonebook *friends;
	if ((friends = (struct phonebook *) malloc (pb_size * sizeof(struct phonebook))) == NULL) //Если память не выделилась - выйти
	{
		perror("Could not allocate initial phonebook memory!\n");
		return -1;
	}
	pb_size_ptr = &pb_size;

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

	strcpy(friends[6].name, "Josh");
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
		printf("\n-------------------------------\n"\
		"---Your friends phonebook---\n"\
		"Choose:\n"\
		"(1) - Show all\n"\
		"(2) - Search by Name\n"\
		"(3) - Search by Number\n"\
		"(4) - Modify existing\n"\
		"(5) - Erase existing\n"\
		"(6) - Add new entry\n"\
		"(0) - Quit\n"\
		"-------------------------------\n");
		scanf("%c", &choice); //Считывание числа выбора в меню
		//Все варианты выбора - ветви цикла Switch. Описание выбора дополнительно дублируется после ввода числа
		switch(choice)
		{
			case '1':
  			printf("(1) - Show all:\n");
				//Передать адресс структуры и текущий размер книги(структуры).
				//Функция сама выведет данные по переданным параметрам
        show_all(friends, pb_size_ptr);
			break;

			case '2':
				printf("(2) - Search by Name\n");
				printf("Enter name(up to 5 case sensitive letters): ");
				scanf("%s", s_name);
				//Передать адресс структуры, текущий размер книги(структуры) и разыскиваемое имя
				//Функция вернёт номер совпавшей записи, иначе -1
				if ((i = search_name(friends, pb_size_ptr, s_name)) != -1)
				{
					printf("\nFound entry: [%d] %s - %d\n", i+1, friends[i].name, friends[i].number);
				}
				else
				{
					printf("\nNo entry found\n");
				}
			break;

			case '3':
				printf("(3) - Search by Number\n");
				printf("Enter number(up to six digits): ");
				scanf("%d", &s_number);
				//Передать адресс структуры, текущий размер книги(структуры) и разыскиваемый номер телефона.
				//Функция вернёт номер совпавшей записи, иначе -1
				if ((i = search_number(friends, pb_size_ptr, s_number)) != -1)
				{
					printf("\nFound entry: [%d] %s - %d\n", i+1, friends[i].name, friends[i].number);
				}
				else
				{
					printf("\nNo entry found\n");
				}
			break;

			case '4':
				printf("(4) - Modify existing\n");
				printf("Choose entry by number [1-%d]: ", pb_size);
				scanf("%d", &i);
				printf("Modifying: [%d] %s - %d\n", i, friends[i-1].name, friends[i-1].number);
				printf("Enter new name(up to 5 letters): ");
				scanf("%s", s_name);
				printf("Enter new number(6 digits): ");
				scanf("%d", &s_number);
				//Передать адресс структуры, номер изменяемой записи, новое имя и новый телефон.
				//Функция вернёт 0 при успехе
				if (modify_entry(friends, i, s_name, s_number) == 0)
				{
					printf("New entry: [%d] %s - %d\n", i, friends[i-1].name, friends[i-1].number);
				}
				else
				{
					printf("Unable to modify entry!\n"); //Этот вывод сейчас недоступен, так как функция пока возвращает только 0
				}
			break;

			case '5':
				printf("(5) - Erase existing\n");
				printf("Choose entry by number [1-%d]: ", pb_size);
				scanf("%d", &i);
				printf("Erasing: [%d] %s - %d\n", i, friends[i-1].name, friends[i-1].number);
				//Передать адресс структуры, текущий размер книги(структуры) и номер удаляемой записи
				//Новый адресс структуры тут же преезаписывается в старую переменную
				friends = (erase_entry(friends, pb_size_ptr, i));
				printf("Succeeded!\n");
			break;

			case '6':
				printf("(6) - Add new entry\n");
				printf("Enter name(up to 5 letters): ");
				scanf("%s", s_name);
				printf("Enter number(up to 6 digits): ");
				scanf("%d", &s_number);
				//Передать адресс структуры, текущий размер книги(структуры) и имя и номер новой записи
				//Новый адресс структуры тут же преезаписывается в старую переменную
				friends = (add_entry(friends, pb_size_ptr, s_name, s_number));
				printf("New entry: [%d] %s - %d\n", pb_size, friends[pb_size-1].name, friends[pb_size-1].number);
			break;

			case '0': //Завершение программы по выбору из меню
  			printf("(0) - Quit\n");
				quit(friends, 0);
			break;

			default: //Вариант по-умолчанию подразумевает, что успешно распознать выбор или сопоставить с доступными варинтами не удалось
  			printf("\nUnrecognized option! Enter one of the options from the list!\n");
			break;
		}
		scanf("%c", &dump); //Забор всех остатоков в вводе консоли, чтобы те не попали в реальные данные
		i = 0;
	}
}
