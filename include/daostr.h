/* str_cat.c */
void str_cat(char *dst, char *s1, char *s2);
/* str_cmp.c */
int str_cmp(char *s1, char *s2);
/* str_decode.c */
void str_decode(unsigned char *str);
/* str_dup.c */
char *str_dup(char *src, int pad);
/* str_folder.c */
void str_folder(char *fpath, char *folder, char *fname);
/* str_fpath.c */
void setdirpath(char *fpath, char *direct, char *fname);
/* str_from.c */
int str_from(char *from, char *addr, char *nick);
/* str_hash.c */
int str_hash(char *str, int seed);
/* str_lower.c */
void str_lower(char *dst, char *src);
/* str_ncmp.c */
int str_ncmp(char *s1, char *s2, int n);
/* str_ncpy.c */
void str_ncpy(char *dst, char *src, int n);
/* str_passwd.c */
char *genpasswd(char *pw);
int chkpasswd(char *passwd, char *test);
/* str_pat.c */
int str_pat(const char *str, const char *pat);
/* str_rle.c */
int str_rle(unsigned char *str);
/* str_stamp.c */
void str_stamp(char *str, time_t *chrono);
/* str_str.c */
char *str_str(char *str, char *tag);
/* str_tail.c */
char *str_tail(char *str);
/* str_time.c */
char *Now(void);
char *Btime(time_t *clock);
char *Ctime(time_t *clock);
char *Etime(time_t *clock);

