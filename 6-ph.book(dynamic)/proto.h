void show_all(struct phonebook *, char *);
int search_name(struct phonebook *, char *, char[5]);
int search_number(struct phonebook *, char *, unsigned int);
int modify_entry(struct phonebook *, int i, char[5], unsigned int);
struct phonebook *erase_entry(struct phonebook *, char *, int);
struct phonebook *add_entry(struct phonebook *, char *, char[5], unsigned int);
int quit(struct phonebook *, int);
