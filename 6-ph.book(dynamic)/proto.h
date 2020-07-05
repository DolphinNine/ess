void show_all(struct phonebook *friends, int pb_size);
int search_name(struct phonebook *friends, int pb_size, char s_name[5]);
int search_number(struct phonebook *friends, int pb_size, unsigned int s_number);
int modify_entry(struct phonebook *friends, int i, char s_name[5], unsigned int s_number);
struct phonebook *erase_entry(struct phonebook *friends, int pb_size, int i);
struct phonebook *add_entry(struct phonebook *friends, int pb_size, char s_name[5], unsigned int s_number);
int quit(struct phonebook *friends, int state);
