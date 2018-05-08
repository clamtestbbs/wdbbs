#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* args.c */
void initsetproctitle(int argc, char **argv, char **envp);
void setproctitle(const char *cmdline);
void printpt(const char *format, ...);
int countproctitle(void);
/* sem.c */
void sem_init(int semkey, int *semid);
void sem_lock(int op, int semid);
/* shm.c */
void *shm_new(int shmkey, int shmsize);
