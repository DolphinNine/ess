struct dirent **get_req(WINDOW *, struct dirent **, int *, int *, char[256]);
int quit(struct dirent **, int *, int);
WINDOW *draw_win(WINDOW *, int, int);
int print_message(WINDOW *, int, char[50]);
int cp_rountine(WINDOW *, char[255], char[PATH_MAX]);
void *paste_file(void *);
void *draw_progress_win(void *);
