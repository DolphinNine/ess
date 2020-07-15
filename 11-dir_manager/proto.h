struct dirent **get_dir(WINDOW *, struct dirent **, int *, char[256]);
int print_dir(WINDOW * wnd, struct dirent **dir_entry, int *entries_amount_ptr, int relative_y);
