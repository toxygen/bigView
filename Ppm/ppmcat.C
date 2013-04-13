/*
  $Log: ppmcat.C,v $
  Revision 1.2  2006/01/18 02:34:05  sandstro
  removed erroneous assert

  Revision 1.1  2004/01/12 18:08:43  sandstro
  *** empty log message ***

  Revision 1.4  2003/11/05 01:05:06  sandstro
  added assert.h

 */

#include <iostream>
#include <sstream>
#include <exception>
#include <stdio.h> // for perror()
#include <strings.h> // for bzero
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <bigFile.h>
#include "ppm.h"

using namespace std;

int
pow2(int exp)
{
  int result = 1;
  while( exp-- > 0 )
    result *= 2;
  return result;
}

void usage(string str)
{
  cerr << "=== " << str << " ===" << endl;
  cerr << "usage: ppmcat -[l|t] -o outfile file1.ppm file2.ppm"<<endl;
  cerr << "     : -l => left to right" << endl;
  cerr << "     : -t => top to bottom" << endl;
  exit(0);
}

main(int argc, char** argv)
{
  int c;
  enum Dir {UNKNOWN,LR,TB};
  Dir dir=UNKNOWN;
  string outfile;
  PPM::Format format[2];
  int fout,fd[2],cpp[2],bpc[2],off[2],imageHeight[2],imageWidth[2];
  
  if( argc==1 ) usage("ppmcat");
  while( (c = getopt(argc,argv,"lto:")) != -1){
    switch( c ){
      case 'o':
	outfile = optarg;
	break;
      case 'l':
	dir=LR;
	break;
      case 't':
	dir=TB;
	break;
    }
  }
  if( dir==UNKNOWN ) usage("no direction specified");
  if( outfile.length()==0 ) usage("no output file specified");

  try {
    PPM::ppmHeader(argv[optind],&format[0],&cpp[0],&bpc[0],
		   &imageWidth[0],&imageHeight[0],&off[0]);
  } catch( exception& e ){throw;}

  try {
    PPM::ppmHeader(argv[optind+1],&format[1],&cpp[1],&bpc[1],
		   &imageWidth[1],&imageHeight[1],&off[1]);
  } catch( exception& e ){throw;}
  
  if( format[0] != format[1] ){
    cerr << "Image formats must be the same" << endl;
    return 0;
  }
  if( cpp[0] != cpp[1] ){
    cerr << "Image pixel sizes must be the same" << endl;
    return 0;
  }
  if( bpc[0] != bpc[1] ){
    cerr << "Image pixel depths must be the same" << endl;
    return 0;
  }
  if( imageWidth[0] != imageWidth[1] && dir==TB){
    cerr << "top=>bottom requires images be the same width" << endl;
    return 0;
  }
  if( imageHeight[0] != imageHeight[1] && dir==LR){
    cerr << "left=>right requires images be the same height" << endl;
    return 0;
  }
  
  int bytesPerPixel = cpp[0] * (bpc[0]/8);
  int componentsPerPixel = cpp[0];
  int bitsPerComponent = bpc[0];

  fd[0] = ::open(argv[optind],O_RDONLY|O_BINARY|O_LARGEFILE);
  fd[1] = ::open(argv[optind+1],O_RDONLY|O_BINARY|O_LARGEFILE);
  fout  = ::open(outfile.c_str(),
		 O_WRONLY|O_BINARY|O_LARGEFILE|O_TRUNC|O_CREAT,0666);
  
  if( fd[0]==-1 ){
    perror("open");
    cerr << "file: " << argv[optind] << endl;
    return 0;
  }
  if( fd[1]==-1 ){
    perror("open");
    cerr << "file: " << argv[optind+1] << endl;
    return 0;
  }
  if( fout==-1 ){
    perror("open");
    cerr << "file: " << outfile << endl;
    return 0;
  }

  if( ! BigFile::seek(fd[0],off[0]) ){
    perror("seek");
    cerr << "file: " << argv[optind] << endl;
    return 0;
  }
  if( ! BigFile::seek(fd[1],off[1]) ){
    perror("seek");
    cerr << "file: " << argv[optind+1] << endl;
    return 0;
  }

  if( dir == LR )
  {
    int res;
    ostringstream ostr;
    int readsize[2]  = {imageWidth[0] * bytesPerPixel,
			imageWidth[1] * bytesPerPixel};
    int outwidth  = imageWidth[0] + imageWidth[1];
    int outheight = imageHeight[0];
    int bufSize   = outwidth * bytesPerPixel;
    int maxValue  = pow2(bitsPerComponent)-1;

    switch( cpp[0] ){
      case 1:
	ostr << "P5" << endl;
	break;
      case 3:
	ostr << "P6" << endl;
	break;
      default:
	usage("unknown format");
	break;
    }
    ostr << outwidth << " " << outheight << " " << maxValue << endl; 
    string hdr = ostr.str();
    if( (res = ::write(fout,hdr.c_str(),hdr.length())) != hdr.length() ){
      perror("write");
      cerr << " res: " << res << endl;
      cerr << "file: " << outfile << endl;
      return 0;
    }

    unsigned char* buf = new unsigned char[bufSize];
    for(int row=0; row<imageHeight[0] ; ++row ){
      if( (res = ::read(fd[0],buf,readsize[0])) != readsize[0] ){
	perror("read");
	cerr << " res: " << res << endl;
	cerr << "file: " << argv[optind] << endl;
	return 0;
      }
      if( (res = ::read(fd[1],&buf[readsize[0]],readsize[1])) != readsize[1] ){
	perror("read");
	cerr << " res: " << res << endl;
	cerr << "file: " << argv[optind+1] << endl;
	return 0;
      }
      if( (res = ::write(fout,buf,bufSize)) != bufSize ){
	perror("write");
	cerr << " res: " << res << endl;
	cerr << "file: " << outfile << endl;
	return 0;
      }
    }
  } 
  else
  {
    int res;
    ostringstream ostr;
    int outwidth  = imageWidth[0];
    int outheight = imageHeight[0] + imageHeight[1];
    int bufSize   = outwidth * bytesPerPixel;
    int maxValue  = pow2(bitsPerComponent)-1;

    switch( cpp[0] ){
      case 1:
	ostr << "P5" << endl;
	break;
      case 3:
	ostr << "P6" << endl;
	break;
      default:
	usage("unknown format");
	break;
    }
    ostr << outwidth << " " << outheight << " " << maxValue << endl; 
    string hdr = ostr.str();
    if( (res = ::write(fout,hdr.c_str(),hdr.length())) != hdr.length() ){
      perror("write");
      cerr << " res: " << res << endl;
      cerr << "file: " << outfile << endl;
      return 0;
    }

    unsigned char* buf = new unsigned char[bufSize];
    for(int row=0; row<imageHeight[0] ; ++row ){
      if( (res = ::read(fd[0],buf,bufSize)) != bufSize ){
	perror("read");
	cerr << " req: " << bufSize << endl;
	cerr << " res: " << res << endl;
	cerr << "file: " << argv[optind] << endl;
	return 0;
      }
      if( (res = ::write(fout,buf,bufSize)) != bufSize ){
	perror("write");
	cerr << " req: " << bufSize << endl;
	cerr << " res: " << res << endl;
	cerr << "file: " << outfile << endl;
	return 0;
      }
    }
    for(int row=0; row<imageHeight[1] ; ++row ){
      if( (res = ::read(fd[1],buf,bufSize)) != bufSize ){
	perror("read");
	cerr << " req: " << bufSize << endl;
	cerr << " res: " << res << endl;
	cerr << "file: " << argv[optind+1] << endl;
	return 0;
      }
      if( (res = ::write(fout,buf,bufSize)) != bufSize ){
	perror("write");
	cerr << " req: " << bufSize << endl;
	cerr << " res: " << res << endl;
	cerr << "file: " << outfile << endl;
	return 0;
      }
    }
  }

  ::close(fd[0]);
  ::close(fd[1]);
  ::close(fout);
}

