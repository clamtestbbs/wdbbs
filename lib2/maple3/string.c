#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "daom3.h"

char * str_add(dst, src)
  char *dst, *src;
{
  while ( (*dst = *src) )
  {
    src++;
    dst++;
  }
  return dst;
}

void str_cat(dst, s1, s2)
  char *dst;
  char *s1;
  char *s2;
{
  while ( (*dst = *s1) )
  {
    s1++;
    dst++;
  }

  while ( (*dst++ = *s2++) )
    ;
}

int str_cmp(s1, s2)
  char *s1, *s2;
{
  int c1, c2, diff;

  do
  {
    c1 = *s1++;
    c2 = *s2++;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;
    if ( (diff = c1 - c2) )
      return (diff);
  } while (c1);
  return 0;
}

/*-------------------------------------------------------*/
/* lib/str_decode.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C for QP/BASE64 decoding		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"				 */
/* ----------------------------------------------------- */


static int qp_code(x)
  register int x;
{
  if (x >= '0' && x <= '9')
    return x - '0';
  if (x >= 'a' && x <= 'f')
    return x - 'a' + 10;
  if (x >= 'A' && x <= 'F')
    return x - 'A' + 10;
  return -1;
}


/* ------------------------------------------------------------------ */
/* BASE64 :							      */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */


static int base64_code(x)
  register int x;
{
  if (x >= 'A' && x <= 'Z')
    return x - 'A';
  if (x >= 'a' && x <= 'z')
    return x - 'a' + 26;
  if (x >= '0' && x <= '9')
    return x - '0' + 52;
  if (x == '+')
    return 62;
  if (x == '/')
    return 63;
  return -1;
}


/* ----------------------------------------------------- */
/* judge & decode QP / BASE64				 */
/* ----------------------------------------------------- */


void str_decode(str)
  unsigned char *str;
{
  int code, c1, c2, c3, c4;
  unsigned char *src, *dst;
  unsigned char buf[256];

#define	IS_QP		0x01
#define	IS_BASE64	0x02
#define	IS_TRAN		0x04

  src = str;
  dst = buf;
  code = 0;

  while ( (c1 = *src++) )
  {
    if (c1 == '?' && *src == '=')
    {
      if (code)
      {
	code &= IS_TRAN;
	if (*++src == ' ')
	  src++;
      }
      continue;
    }

    if (c1 == '\n')		/* chuan: multi line encoding */
      goto line;

    if (code & (IS_QP | IS_BASE64))
    {
      if (c1 == '_')
      {
	*dst++ = ' ';
	continue;
      }

      if ((c1 == '=') && (code & IS_QP))
      {
	if (!(c1 = *src))
	  break;

	if (!(c2 = *++src))
	  break;

	src++;
	*dst++ = (qp_code(c1) << 4) | qp_code(c2);
	code |= IS_TRAN;
	continue;
      }

      if (code & IS_BASE64)
      {
	c2 = *src++;

	if (c1 == '=' || c2 == '=')
	{
	  code ^= IS_BASE64;
	  continue;
	}

	if (!(c3 = *src++))
	  break;

	if (!(c4 = *src++))
	  break;

	c2 = base64_code(c2);
	*dst++ = (base64_code(c1) << 2) | ((c2 & 0x30) >> 4);

	if (c3 == '=')
	{
	  code ^= IS_BASE64;
	}
	else
	{
	  c3 = base64_code(c3);
	  *dst++ = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);

	  if (c4 == '=')
	    code ^= IS_BASE64;
	  else
	    *dst++ = ((c3 & 0x03) << 6) | base64_code(c4);
	}

	code |= IS_TRAN;
	continue;
      }
    }

    /* "=?%s?Q?" for QP, "=?%s?B?" for BASE64 */

    if ((c1 == '=') && (*src == '?'))
    {
      c2 = 0;

      for (;;)
      {
	c1 = *++src;
	if (!c1)
	  goto home;

	if (c1 == '?')
	{
	  if (++c2 >= 2)
	    break;

	  continue;
	}

	if (c2 == 1)
	{
	  if (c1 == 'Q')
	    code = IS_QP;
	  else if (c1 == 'B')
	    code = IS_BASE64;
	}
      }

      src++;
      continue;
    }

    *dst++ = c1;
  }

home:

  if (code & IS_TRAN)
  {
line:
    *dst = '\0';
    strcpy(str, buf);
  }
}
	
char * str_dup(src, pad)
  char *src;
  int pad;
{
  char *dst;

  dst = (char *) malloc(strlen(src) + pad);
  strcpy(dst, src);
  return dst;
}

void str_folder(fpath, folder, fname)
  char *fpath;
  char *folder;
  char *fname;
{
  int ch;
  char *token;

  while ( (ch = *folder++) )
  {
    *fpath++ = ch;
    if (ch == '/')
      token = fpath;
  }
  if (*token != '.')
    token -= 2;
  strcpy(token, fname);
}

void setdirpath(fpath, direct, fname)
  char *fpath, *direct, *fname;
{
  int ch;
  char *target;
      
  while ( (ch = *direct) )
  {
    *fpath++ = ch;
    if (ch == '/')
      target = fpath;
    direct++;
  }
                              
  strcpy(target, fname);
}

/* ----------------------------------------------------  */
/* E-mail address format				 */
/* ----------------------------------------------------  */
/* 1. user@domain					 */
/* 2. <user@domain>					 */
/* 3. user@domain (nick)				 */
/* 4. user@domain ("nick")				 */
/* 5. nick <user@domain>				 */
/* 6. "nick" <user@domain>				 */
/* ----------------------------------------------------  */


int str_from(from, addr, nick)
  char *from, *addr, *nick;
{
  char *str, *ptr, *langle;
  int cc;

  *nick = 0;

  langle = ptr = NULL;

  for (str = from; (cc = *str); str++)
  {
    if (cc == '<')
      langle = str;
    else if (cc == '@')
      ptr = str;
  }

  if (ptr == NULL)
  {
    strcpy(addr, from);
    return -1;
  }

  if (langle && langle < ptr && str[-1] == '>')
  {
    /* case 2/5/6 : name <mail_addr> */

    str[-1] = 0;
    if (langle > from)
    {
      ptr = langle - 2;
      if (*from == '"')
      {
	from++;
	if (*ptr == '"')
	  ptr--;
      }
      if (*from == '(')
      {
	from++;
	if (*ptr == ')')
	  ptr--;
      }
      ptr[1] = '\0';
      strcpy(nick, from);
      str_decode(nick);
    }

    from = langle + 1;
  }
  else
  {
    /* case 1/3/4 */

    if (*--str == ')')
    {
      if (str[-1] == '"')
	str--;
      *str = 0;

      if ( (ptr = (char *) strchr(from, '(')) )
      {
	ptr[-1] = 0;
	if (*++ptr == '"')
	  ptr++;

	strcpy(nick, ptr);
	str_decode(nick);
      }
    }
  }

  strcpy(addr, from);
  return 0;
}

int str_has(list, tag)
  char *list;
  char *tag;
{
  int cc, len;

    len = strlen(tag);
    for (;;)
    {
      cc = list[len];
      if ((!cc || cc == '/') && !str_ncmp(list, tag, len))
	return 1;

      for (;;)
      {
	cc = *list;
	if (!cc)
	  return 0;
	list++;
	if (cc == '/')
	  break;
      }
    }
}

int str_hash(str, seed)
  char *str;
  int seed;
{
  int cc;

  while ( (cc = *str++) )
  {
    seed = (seed << 5) - seed + cc;	/* 31 * seed + cc */
  }
  return (seed & 0x7fffffff);
}

int str_len(str)
  char *str;
{
  int cc, len;

  for (len = 0; (cc = *str); str++)
  {
    if (cc != ' ')
      len++;
  }

  return len;
}

void str_lower(dst, src)
  char *dst, *src;
{
  int ch;

  do
  {
    ch = *src++;
    if (ch >= 'A' && ch <= 'Z')
      ch |= 0x20;
    *dst++ = ch;
  } while (ch);
}

int str_ncmp(s1, s2, n)
  char *s1, *s2;
  int n;
{
  int c1, c2;

  while (n--)
  {
    c1 = *s1++;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;

    c2 = *s2++;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;

    if (c1 -= c2)
      return (c1);

    if (!c2)
      break;
  }

  return 0;
}


/*
 * str_ncpy() - similar to strncpy(3) but terminates string always with '\0'
 * if n != 0, and doesn't do padding
 */

void str_ncpy(dst, src, n)
  char *dst;
  char *src;
  int n;
{
  char *end;

  end = dst + n - 1;

  do
  {
    n = (dst >= end) ? 0 : *src++;
    *dst++ = n;
  } while (n);
}

char *str_ndup(src, len)
  char *src;
  int len;
{
  char *dst, *str, *end;

  str = src;
  end = src + len;
  do
  {
    if (!*str++)
    {
      end = str;
      break;
    }
  } while (str < end);

  dst = (char *) malloc(end - src);

  for (str = dst; (*str = *src); src++)
  {
    str++;
    if (src >= end)
    {
      *str = '\0';
      break;
    }
  }

  return dst;
}

#ifndef	PASSLEN
#define	PASSLEN 14
#endif

/* ----------------------------------------------------- */
/* password encryption					 */
/* ----------------------------------------------------- */

char *crypt();
static char pwbuf[PASSLEN];


char * genpasswd(pw)
  char *pw;
{
  char saltc[2];
  int i, c;

  if (!*pw)
    return pw;

  i = 9 * getpid();
  saltc[0] = i & 077;
  saltc[1] = (i >> 6) & 077;

  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
    if (c > '9')
      c += 7;
    if (c > 'Z')
      c += 6;
    saltc[i] = c;
  }
  strcpy(pwbuf, pw);
  return crypt(pwbuf, saltc);
}


int checkpasswd(passwd, test)
  char *passwd, *test;
{
  char *pw;

  str_ncpy(pwbuf, test, PASSLEN);
  pw = crypt(pwbuf, passwd);
  return (strncmp(pw, passwd, PASSLEN));
}
/* str_pat : wild card string pattern match support ? * \ */


int str_pat(str, pat)
  const char *str;
  const char *pat;
{
  const char *xstr, *xpat;
  int cs, cp;

  xpat = NULL;

  while ( (cs = *str) )
  {
    cp = *pat++;
    if (cp == '*')
    {
      for (;;)
      {
	cp = *pat;

	if (cp == '\0')
	  return 1;

	pat++;

	if (cp != '*')
	{
	  xpat = pat;
	  xstr = str;
	  break;
	}
      }
    }

    str++;

    if (cp == '?')
      continue;

    if (cp == '\\')
      cp = *pat++;

#ifdef	CASE_IN_SENSITIVE
    if (cp >= 'A' && cp <= 'Z')
      cp += 0x20;

    if (cs >= 'A' && cs <= 'Z')
      cs += 0x20;
#endif

    if (cp == cs)
      continue;

    if (xpat == NULL)
      return 0;

    pat = xpat;
    str = ++xstr;
  }

  while ( (cp = *pat) )
  {
    if (cp != '*')
      return 0;

    pat++;
  }

  return 1;
}


#ifdef	TEST
#define	STR_PAT(x, y)	printf("<%s, %s> : %d\n", x, y, str_pat(x, y))


main()
{
  STR_PAT("a", "a*");
  STR_PAT("abc", "a*");
  STR_PAT("abc", "a*c");
  STR_PAT("abc", "a?c");
  STR_PAT("level", "l*l");
  STR_PAT("level", "l*e*l");
  STR_PAT("lelelelel", "l*l*l*l");
}
#endif	/* TEST */
/* reverse the string */


char * str_rev(dst, src)
  char *dst, *src;
{
  int cc;

  *dst = '\0';

  while ( (cc = *src) )
  {
    *--dst = cc;
    src++;
  }
  return dst;
}

int str_rle(str)			/* run-length encoding */
  unsigned char *str;
{
  unsigned char *src, *dst;
  int cc, rl;

  dst = src = str;
  while ( (cc = *src++) )
  {
    if (cc > 8 && cc == src[0] && cc == src[1] && cc == src[2])
    {
      src += 2;
      rl = 4;
      while (*++src == cc)
      {
	if (++rl >= 255)
	  break;
      }

      *dst++ = 8;
      *dst++ = rl;
    }
    *dst++ = cc;
  }

  *dst = '\0';
  return dst - str;
}

/* ------------------------------------------ */
/* mail / post 時，依據時間建立檔案，加上郵戳 */
/* ------------------------------------------ */
/* Input: fpath = directory;		      */
/* Output: fpath = full path;		      */
/* ------------------------------------------ */

void str_stamp(str, chrono)
  char *str;
  time_t *chrono;
{
  struct tm *ptime;

  ptime = localtime(chrono);
  sprintf(str, "%02d/%02d/%02d",
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
}
#ifndef	NULL
#define	NULL	(char *) 0
#endif


char * str_str(str, tag) 
  char *str; 
  char *tag;                  /* non-empty lower case pattern */ 
{ 
  int cc, c1, c2;
  char *p1, *p2;

  cc = *tag++;
 
  while ( (c1 = *str) )
  {
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 0x20;

    if (c1 == cc)
    {
      p1 = str;
      p2 = tag;

      do
      {
        c2 = *p2;
        if (!c2)
          return str;
 
        p2++;
        c1 = *++p1;
        if (c1 >= 'A' && c1 <= 'Z')
          c1 |= 0x20;
      } while (c1 == c2);
    }
 
    str++;
  }

  return NULL;
}


#if 0
int
str_str(str, tag)
  char *str, *tag;		/* tag : lower-case string */
{
  int ch, key, chk;
  char *s, *t;

  key = *tag++;
  if (!key)
    return 0;

  while (ch = *str)
  {
    str++;
    if (ch >= 'A' && ch <= 'Z')
      ch |= 0x20;
    if (ch == key)
    {
      s = str;
      t = tag;
      str = NULL;
      for (;;)
      {
	chk = *t;
	if (!chk)
	  return 1;

	ch = *s;
	if (ch >= 'A' && ch <= 'Z')
	  ch |= 0x20;

	if (ch != chk)
	  break;

	if (str == NULL)
	{
	  if (ch == key)
	    str = s;
	}

	s++;
	t++;
      }
      if (str == NULL)
	str = s;
    }
  }
  return 0;
}
#endif


#if 0
int
str_str(str, tag)
  char *str, *tag;		/* tag : lower-case string */
{
  char buf[STRLEN];

  str_lower(buf, str);
  return (int) strstr(buf, tag);
}
#endif

char * str_tail(str)
  char *str;
{
  while (*str)
  {
    str++;
  }
  return str;
}

static char datemsg[32];


char * Btime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);

  sprintf(datemsg, "%02d/%02d/%02d%3d:%02d:%02d ",
    t->tm_year % 100, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
  return (datemsg);
}


char * Ctime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);
  static char week[] = "日一二三四五六";

  sprintf(datemsg, "%d年%2d月%2d日%3d:%02d:%02d 星期%.2s",
    t->tm_year - 11, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec, &week[t->tm_wday << 1]);
  return (datemsg);
}


char * Etime(clock)
  time_t *clock;
{
  strftime(datemsg, 22, "%D %T %a", localtime(clock));
  return (datemsg);
}


char * Now()
{
  time_t now;

  time(&now);
  return Btime(&now);
}

/*------------------------------------------------------------------ */
/* Remove(strip) left and right English and Chinese blanks in a line */
/* ----------------------------------------------------------------- */

char* trim2(char* buffer)
{
   int j;

   for (j = 0; buffer[j] == ' ' || strncmp(buffer + j, "　", 2) == 0; j++)
      if(strncmp(buffer + j, "　", 2) == 0)
         j++;
   memmove(buffer, buffer + j, strlen(buffer + j) + 1);
   for (j = strlen(buffer);
        buffer[j - 1] == ' ' || strncmp(buffer + j - 1, "　", 2) == 0; j--)
      if(strncmp(buffer + j - 1, "　", 2) == 0)
         j--;
   buffer[j] = 0;
   return buffer;
}

void str_trim(buf)			/* remove trailing space */
  char *buf;
{
  char *p = buf;

  while (*p)
    p++;
  while (--p >= buf)
  {
    if (*p == ' ')
      *p = '\0';
    else
      break;
  }
}

char * str_ttl(title)
  char *title;
{
  if (title[0] == 'R' && title[1] == 'e' && title[2] == ':')
  {
    title += 3;
    if (*title == ' ')
      title++;
  }

  return title;
}

