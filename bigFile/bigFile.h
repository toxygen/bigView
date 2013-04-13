//////////////////////////////////////////////////////////////////////////
/////////////////////////////// bigFile.h ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef _BIG_FILE_H_
#define _BIG_FILE_H_

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

namespace BigFile {
  typedef long long Offset;                   // 64 bit offset
  bool seek(int fd, BigFile::Offset offset);  // go to a position
  bool filesize(int fd, BigFile::Offset* fsize);
  BigFile::Offset tell(int fd);               // get current position
} // namespace BigFile

#endif
