//////////////////////////////////////////////////////////////////////////
/////////////////////////////// bigFile.C ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h> // for SEEK_SET
#include "bigFile.h"

#if __linux && !defined(_XBS5_LPBIG_OFFBIG)
#include <linux/unistd.h>
_syscall5(int,_llseek,uint,fd,ulong,hi,ulong,lo,loff_t *,res,uint,wh);
#endif

BigFile::Offset
BigFile::tell(int fd)
{
  off_t res=0;
  BigFile::Offset offset=0;
#if defined(_XBS5_LPBIG_OFFBIG)
  offset = lseek(fd, 0, SEEK_CUR);
#elif defined(__linux)
  loff_t endoff;
  res = _llseek(fd, 0, 0, &endoff, SEEK_CUR);
  if( res == 0 ) offset = endoff;
#elif defined(__sgi) || defined(__sun)
  offset = lseek64(fd, 0, SEEK_CUR);
#else
  res = lseek(fd, 0, SEEK_CUR);
  if( res==offset ) offset = res;
#endif
  return offset;
}

bool
BigFile::filesize(int fd, BigFile::Offset* fsize)
{
  off_t res=0;
  BigFile::Offset curpos=BigFile::tell(fd);
#if defined(_XBS5_LPBIG_OFFBIG)
  res = lseek(fd, 0, SEEK_END);
  BigFile::seek(fd,curpos);
  *fsize = res;
  return res>0 ? true : false;
#elif defined(__linux)
  loff_t endoff;  
  res = _llseek(fd, 0, 0, fsize, SEEK_END);
  BigFile::seek(fd,curpos);
  return res==0 ? true : false;
#elif defined(__sgi) || defined(__sun)
  res = lseek64(fd, 0, SEEK_END);
  BigFile::seek(fd,curpos);
  *fsize = res;
  return res>0 ? true : false;
#else
  res = lseek(fd, 0, SEEK_END);
  BigFile::seek(fd,curpos);
  *fsize = res;
  return res>0 ? true : false;
#endif
}

bool
BigFile::seek(int fd, BigFile::Offset offset)
{
  off_t res=0;
#if defined(_XBS5_LPBIG_OFFBIG)
  res = lseek(fd, offset, SEEK_SET);
  return res==offset ? true : false;
#elif defined(__linux)
  loff_t endoff;  
  res = _llseek(fd, 
		offset>>32, 
		offset & (0xffffffff), 
		&endoff, SEEK_SET);

  return res==0 ? true : false;

#elif defined(__sgi) || defined(__sun)
  res = lseek64(fd, offset, SEEK_SET);
  return res==offset ? true : false;
#else
  res = lseek(fd, offset, SEEK_SET);
  return res==offset ? true : false;
#endif
}
