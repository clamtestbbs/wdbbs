/* setpf.c */
void setadir(char *buf, char *path);
void setapath(char *buf, char *boardname);
void setbfile(char *buf, char *boardname, char *fname);
void setbgdir(char *buf, char *boardname);
void setbpath(char *buf, char *boardname);
void sethomedir(char *buf, char *userid);
void sethomefile(char *buf, char *userid, char *fname);
void sethomeman(char *buf, char *userid);
void sethomepath(char *buf, char *userid);
/* strip_ansi.c */
int strip_ansi(char *buf, char *str, int mode);
/* ci_strncmp.c */
int ci_strncmp(register char *s1, register char *s2, register int n);
/* cut_ansistr.c */
int cut_ansistr(char *buf, char *str, int len);
/* isprint2.c */
int isprint2(int ch);
/* not_alnum.c */
int not_alnum(int ch);
/* not_alpha.c */
int not_alpha(int ch);
/* strstr_lower.c */
int strstr_lower(char *str, char *tag);
/* friend_count.c */
int friend_count(char *fname);
/* nextfield.c */
char *nextfield(register char *data, register char *field);
/* bad_user.c */
int bad_user(char *name);
/* counter.c */
void counter(char *filename, char *modes, int n);
/* file_list_count.c */
int file_list_count(char *fname);
/* b_suckinfile.c */
void b_suckinfile(FILE *fp, char *fname);
/* url_encode.c */
void url_encode(unsigned char *dst, unsigned char *src);
