#ifndef	_DAO_M3_H_
#define	_DAO_M3_H_

#include <stdio.h>
#include <sys/types.h>
#include "hdr.h"			/* prototype */

#ifndef	NULL
#define	NULL	0			/* ((char *) 0) */
#endif

#ifndef	BLK_SIZ
#define BLK_SIZ         4096		/* disk I/O block size */
#endif

#ifndef	REC_SIZ
#define REC_SIZ         512		/* disk I/O record size */
#endif

/* file.c */
void f_cat(char *fpath, char *msg);
int f_cp(char *src, char *dst, int mode);
char *f_img(char *fpath, int *fsize);
int f_ln(char *src, char *dst);
int f_exlock(int fd);
int f_unlock(int fd);
char *f_map(char *fpath, int *fsize);
int f_mode(char *fpath);
int f_mv(char *src, char *dst);
FILE *f_new(char *fold, char *fnew);
int f_open(char *fpath);
void brd_fpath(char *fpath, char *board, char *fname);
void gem_fpath(char *fpath, char *board, char *fname);
void usr_fpath(char *fpath, char *user, char *fname);
int f_rm(char *fpath);
void f_suck(FILE *fp, char *fpath);

/* isnot.c */
int is_alnum(int ch);
int is_alpha(int ch);
int is_fname(char *str);
int is_fpath(char *path);
int not_addr(char *addr);

/* record.c */
int rec_add(char *fpath, void *data, int size);
int rec_apply(char *fpath, int (*fptr)(void), int size);
int rec_del(char *data, int size, int pos, int (*fchk)(void), int (*fdel)(void));
int rec_get(char *fpath, void *data, int size, int pos);
int rec_ins(char *fpath, void *data, int size, int pos, int num);
int rec_mov(char *data, int size, int from, int to);
int rec_num(char *fpath, int size);
int rec_put(char *fpath, void *data, int size, int pos);

/* string.c */
char *str_add(char *dst, char *src);
void str_cat(char *dst, char *s1, char *s2);
int str_cmp(char *s1, char *s2);
void str_decode(unsigned char *str);
char *str_dup(char *src, int pad);
void str_folder(char *fpath, char *folder, char *fname);
void setdirpath(char *fpath, char *direct, char *fname);
int str_from(char *from, char *addr, char *nick);
int str_has(char *list, char *tag);
int str_hash(char *str, int seed);
int str_len(char *str);
void str_lower(char *dst, char *src);
int str_ncmp(char *s1, char *s2, int n);
void str_ncpy(char *dst, char *src, int n);
char *str_ndup(char *src, int len);
char *genpasswd(char *pw);
int checkpasswd(char *passwd, char *test);
int str_pat(const char *str, const char *pat);
char *str_rev(char *dst, char *src);
int str_rle(unsigned char *str);
void str_stamp(char *str, time_t *chrono);
char *str_str(char *str, char *tag);
char *str_tail(char *str);
char *Btime(time_t *clock);
char *Ctime(time_t *clock);
char *Etime(time_t *clock);
char *Now(void);
char *trim2(char *buffer);
void str_trim(char *buf);
char *str_ttl(char *title);

/* attr_lib.c */
int attr_get(char *userid, int key, void *value);
int attr_put(char *userid, int key, void *value);
int attr_step(char *userid, int key, int dflt, int step);

/* dl_lib.c */
void log_dl(char *mode, char *msg);
void *DL_get(char *name);
int DL_func(char *name, ...);

#endif   // _DAO_M3_H_
