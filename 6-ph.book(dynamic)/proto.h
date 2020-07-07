void show_all(struct phonebook *friends, char *pb_size_ptr);
int search_name(struct phonebook *friends, char *pb_size_ptr, char s_name[5]);
int search_number(struct phonebook *friends, char *pb_size_ptr, unsigned int s_number);
int modify_entry(struct phonebook *friends, int i, char s_name[5], unsigned int s_number);
struct phonebook *erase_entry(struct phonebook *friends, char *pb_size_ptr, int i);
struct phonebook *add_entry(struct phonebook *friends, char *pb_size_ptr, char s_name[5], unsigned int s_number);
int quit(struct phonebook *friends, int state);
