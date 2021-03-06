/*-------------------------------------------------------*/
/* util/topquery.c        ( NTHU CS MapleBBS Ver 2.36 )  */
/*-------------------------------------------------------*/
/* target : 使用者 人氣/好奇 排行榜                      */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#define REAL_INFO

struct manrec
{
  char userid[IDLEN+1];
  char username[23];
  usint toquery;
  usint bequery;
};
typedef struct manrec manrec;
struct manrec allman[MAXUSERS];

userec aman;
int num;
FILE *fp;

#define TYPE_TO       0
#define TYPE_BE       1


int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if ((ptr = strtok(buf, " \t\n\r")) && !strcasecmp(ptr, key))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}

int
to_cmp(b, a)
  struct manrec *a, *b;
{
  return (a->toquery - b->toquery);
}


int
be_cmp(b, a)
  struct manrec *a, *b;
{
  return (a->bequery - b->bequery);
}


void
top(type)
{
  static char *str_type[2] = {" 好奇度 ", " 人氣度 ",};
  int i, j, rows = (num + 1) / 2;
  char buf1[80], buf2[80];

if(type == 1)
  fprintf(fp,"\n\n");

  fprintf(fp, "\
[1;33m○──────────┬→ [%dm    %8.8s排行榜     [33;40m ←┬──────────○[m\n\
[1;37m名次[33m─[37m代號[33m───[37m暱稱[33m─┴─────[37m數目   名次[33m─[37m代號[33m─┴─[37m暱稱[33m──────[37m數目[m\
", (type*type) + 41, str_type[type]);
  for (i = 0; i < rows; i++)
  {
    sprintf(buf1, "[%2d] %-11.11s%-16.16s%5d",
      i + 1, allman[i].userid, allman[i].username,
      (type == 1 ? allman[i].bequery : allman[i].toquery));
    j = i + rows;
    sprintf(buf2, "[%2d] %-11.11s%-16.16s%4d",
      j + 1, allman[j].userid, allman[j].username,
      (type == 1 ? allman[j].bequery : allman[j].toquery));
    if (i < 3)
      fprintf(fp, "\n [1;%dm%-40s[0;37m%s", 31 + i, buf1, buf2);
    else
      fprintf(fp, "\n %-40s%s", buf1, buf2);
  }
}


int
not_alpha(ch)
  register char ch;
{
  return (ch < 'A' || (ch > 'Z' && ch < 'a') || ch > 'z');
}

int
not_alnum(ch)
  register char ch;
{
  return (ch < '0' || (ch > '9' && ch < 'A') ||
    (ch > 'Z' && ch < 'a') || ch > 'z');
}

int
bad_user_id(userid)
  char *userid;
{
  register char ch;
  if (strlen(userid) < 2)
    return 1;
  if (not_alpha(*userid))
    return 1;
  while (ch = *(++userid))
  {
    if (not_alnum(ch))
      return 1;
  }
  return 0;
}

main(argc, argv)
  int argc;
  char **argv;
{
  FILE *inf;
  int i;

  if (argc < 3)
  {
    printf("Usage: %s <num_top> <out-file>\n", argv[0]);
    exit(1);
  }

  num = atoi(argv[1]);
  if (num == 0)
    num = 30;

  inf = fopen(BBSHOME "/.PASSWDS", "rb");

  if (inf == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    exit(0);
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf); i++)
  {
    if(belong("etc/nontop",aman.userid) || bad_user_id(aman.userid)
       || strchr(aman.userid,'.'))
    {
        i--;
    }
   else
    {
     strcpy(allman[i].userid, aman.userid); 
     strncpy(allman[i].username,aman.username,23); 
     allman[i].toquery = aman.toquery; 
     allman[i].bequery = aman.bequery; 
    }
  }

  if ((fp = fopen(argv[2], "w")) == NULL)
  {
    printf("cann't open topquery\n");
    return 0;
  }

  qsort(allman, i, sizeof(manrec), to_cmp);
  top(TYPE_TO);

  qsort(allman, i, sizeof(manrec), be_cmp);
  top(TYPE_BE);

  fclose(inf);
  fclose(fp);
}
