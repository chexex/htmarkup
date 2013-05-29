#if defined(linux)
/* pread
 */
#  define _XOPEN_SOURCE 500
#endif

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "fileutils.hpp"

bool file_exist (const char *pathname)
{
  return
      (access (pathname, F_OK) != -1);
}

bool Write( int fd, const void *data, size_t len )
{
  const char *pd = reinterpret_cast< const char * >( data );
  while ( len > 0 )
  {
    const ssize_t n = write(fd, pd, len);
    if ( n < 0 )
    {
      switch (errno)
      {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          return false;
      }
    }

    pd  += n;
    len -= n;
  }

  return true;
}

bool Read (int fd, void *data, size_t len)
{
  ssize_t n;
  char *pb = (char *)(data);
    
  while (len > 0) {
    n = read(fd, pb, len);
    if (!n)
      return false;
    
    if (n < 0) 
      switch (errno) {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          return false;
      }

      pb  += n;
      len -= n;
  }

  return true;
}

#if HAVE_PREAD

bool PRead (int fd, void *buf, size_t len, off_t offset)
{
  ssize_t n;
  char *pb = (char *)buf;
    
  while (len) {
    n = pread (fd, pb, len, offset);
    if (!n)
      return false; // read at EOF point
    
    if (n < 0) {
      switch (errno) {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          return false;
      }
    }
        
    offset += n;
    pb     += n;
    len    -= n;
  }
    
  return true;
}

#endif

off_t file_size(int fd)
{
  struct stat st;
  
  if (fstat(fd, &st) == -1)
    return -1;
  return st.st_size;
}

off_t file_size(const char *path)
{
  int fd;
  off_t ret;
  
  fd = open(path, O_RDONLY);
  if (fd == -1)
    return (off_t)(-1);
  
  ret = file_size(fd);
  close(fd);
  return ret;
}
