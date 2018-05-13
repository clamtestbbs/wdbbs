#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

int
rec_add(fpath, data, size)
  char *fpath;
  void *data;
  int size;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0)
    return -1;

// wildcat : 還是這樣比較保險
  flock(fd, LOCK_EX);
  lseek(fd, 0, SEEK_END);
  
  write(fd, data, size);

  flock(fd, LOCK_UN);
  close(fd);

  return 0;
}
#include "dao.h"
#include <fcntl.h>
#include <unistd.h>

int
rec_apply(fpath, fptr, size)
  char *fpath;
  int (*fptr) ();
  int size;
{
  char buf[REC_SIZ];
  int fd;

  if ((fd = open(fpath, O_RDONLY)) == -1)
    return -1;

  while (read(fd, buf, size) == size)
  {
    if ((*fptr) (buf))
    {
      close(fd);
      return -2;
    }
  }
  close(fd);
  return 0;
}
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dao.h"

int
rec_del(data, size, pos, fchk, fdel)
  char *data;
  int size;
  int pos;
  int (*fchk) ();
  int (*fdel) ();
{
  int fd;
  off_t off, len;
  char pool[REC_SIZ];
  struct stat st;

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

#ifdef	SOLARIS				/* lkchu: solaris 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else
  flock(fd, LOCK_EX);
#endif

  fstat(fd, &st);
  len = st.st_size;

  data = pool;
  off = size * pos;

  /* 驗證 pos 位置資料的正確性 */

  if (len > off)
  {
    lseek(fd, off, SEEK_SET);
    read(fd, data, size);

    pos = fchk ? (*fchk) (data) : 1;
  }
  else
  {
    pos = 0;
  }

  /* 不對的話，重頭找起 */

  if (!pos)
  {
    off = 0;
    lseek(fd, off, SEEK_SET);
    while (read(fd, data, size) == size)
    {
      if ( ( pos = (*fchk) (data) ) )
	break;

      off += size;
    }
  }

  /* 找到之後，刪除資料 */

  if (pos)
  {
    if (fdel)
      pos = (*fdel) (data);

    if (pos)
    {
      /* really delete it */

      len -= (off + size);
      data = (char *) malloc(len);
      read(fd, data, len);
    }
    else
    {
      /* just update it */

      len = size;
    }

    lseek(fd, off, SEEK_SET);
    write(fd, data, len);

    if (pos)
    {
      ftruncate(fd, off + len);
      free(data);
    }
  }

#ifdef	SOLARIS				/* lkchu: solaris 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else  
  flock(fd, LOCK_UN);
#endif
  close(fd);

  return 0;
}
#include <fcntl.h>
#include <unistd.h>


int
rec_get(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;
  int ret;

  ret = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    if (lseek(fd, (off_t) (size * (pos-1)), SEEK_SET) >= 0)
    {
      if (read(fd, data, size) == size)
      {
	close(fd);
	ret = 0;
	return 0;
      }
    }
    close(fd);
  }
  return ret;
}
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

int
rec_ins(fpath, data, size, pos, num)
  char *fpath;
  void *data;
  int size;
  int pos;
  int num;
{
  int fd;
  off_t off, len;
  struct stat st;

  if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
    return -1;

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else  
  flock(fd, LOCK_EX);
#endif

  fstat(fd, &st);
  len = st.st_size;

  off = size * pos;
  lseek(fd, off, SEEK_SET);

  size *= num;
  len -= off;
  if (len > 0)
  {
    fpath = (char *) malloc(pos = len + size);
    memcpy(fpath, data, size);
    read(fd, fpath + size, len);
    lseek(fd, off, SEEK_SET);
    data = fpath;
    size = pos;
  }

  write(fd, data, size);

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else
  flock(fd, LOCK_UN);
#endif
  close(fd);

  if (len > 0)
    free(data);

  return 0;
}
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

int
rec_mov(data, size, from, to)
  char *data;
  int size;
  int from;
  int to;
{
  int fd, backward;
  off_t off, len;
  char *pool;
  struct stat st;

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

#ifdef  SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else
  flock(fd, LOCK_EX);
#endif

  fstat(fd, &st);
  len = st.st_size / size - 1;

  if (from > to)
  {
    backward = from;
    from = to;
    to = backward;
    backward = 1;
  }
  else
  {
    backward = 0;
  }

  if (to >= len)
    to = len;

  off = size * from;
  lseek(fd, off, SEEK_SET);

  len = (to - from + 1) * size;
  pool = data = (char *) malloc(len + size);

  if (backward)
    data += size;
  read(fd, data, len);

  data = pool + len;
  if (backward)
    memcpy(pool, data, size);
  else
    memcpy(data, pool, size);

  data = pool;
  if (!backward)
    data += size;

  lseek(fd, off, SEEK_SET);
  write(fd, data, len);

#ifdef  SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else
  flock(fd, LOCK_UN);
#endif
    
  close(fd);
  free(pool);

  return 0;
}
#include <sys/stat.h>


int
rec_num(fpath, size)
  char *fpath;
  int size;
{
  struct stat st;

  if (stat(fpath, &st) == -1)
    return 0;
  return (st.st_size / size);
}
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>


int
rec_put(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT, 0600)) < 0)
    return -1;

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else  
  flock(fd, LOCK_EX);
#endif

  lseek(fd, (off_t) (size * pos), SEEK_SET);
  write(fd, data, size);

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else
  flock(fd, LOCK_UN);
#endif

  close(fd);

  return 0;
}
