#include <stdio.h>

/* f_cat.c */
void f_cat(char *fpath, char *msg);
/* f_cp.c */
int f_cp(char *src, char *dst, int mode);
/* f_img.c */
char *f_img(char *fpath, int *fsize);
/* f_ln.c */
int f_ln(char *src, char *dst);
/* f_map.c */
char *f_map(char *fpath, int *fsize);
/* f_mode.c */
int f_mode(char *fpath);
/* f_mv.c */
int f_mv(char *src, char *dst);
/* f_new.c */
FILE *f_new(char *fold, char *fnew);
/* f_open.c */
int f_open(char *fpath);
/* f_path.c */
void brd_fpath(char *fpath, char *board, char *fname);
void gem_fpath(char *fpath, char *board, char *fname);
void usr_fpath(char *fpath, char *user, char *fname);
/* f_rm.c */
int f_rm(char *fpath);
/* f_suck.c */
void f_suck(FILE *fp, char *fpath);
/* f_lock.c */
int f_exlock(int fd);
int f_unlock(int fd);
/* attr_lib.c */
int attr_get(char *userid, int key, void *value);
int attr_put(char *userid, int key, void *value);
int attr_step(char *userid, int key, int dflt, int step);
/* dl_lib.c */
void log_dl(char *mode, char *msg);
void *DL_get(char *name);
int DL_func(char *name, ...);
