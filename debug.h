#ifndef _DEBUG_H_

#include <iostream>    // for cout, endl
#include <iomanip>
#include "ostr.h"

#ifdef DEBUG 

# define FANCYMESG(m) { \
   std::cout<<"===== "<<m<<" ====="<<std::endl; \
}
# define FANCYVAR(v) { \
   std::cout<<"===== "<<#v<<" = "<<v<<" ====="<<std::endl; \
}
# define FANCYVAR2(v1,v2) { \
   std::cout<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"====="<<std::endl; \
}
# define FANCYVAR3(v1,v2,v3) { \
   std::cout<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"====="<<std::endl; \
}
# define FANCYVAR4(v1,v2,v3,v4) { \
   std::cout<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"," \
            <<#v4<<"="<<v4<<"====="<<std::endl; \
}
# define MESG(m) { \
   std::cout << m << std::endl; \
}
# define MESGVAR(m,v) { \
   std::cout<<m<<":"<<#v<<" = "<<v<<std::endl; \
}
# define MESGVAR2(m,v1,v2) { \
   std::cout<<m<<":["<<#v1<<","<<#v2<<"]=" \
            <<v1<<","<<v2<<std::endl; \
}
# define MESGVAR3(m,v1,v2,v3) { \
   std::cout<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<"]=" \
           <<v1<<","<<v2<<","<<v3<<std::endl; \
}
# define MESGVAR4(m,v1,v2,v3,v4) { \
   std::cout<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]=" \
           <<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl; \
}
# define MESGVARHEX(m,v) { \
   std::cout<<m<<":"<<#v<<" = 0x" \
            <<std::hex<<(void*)v \
            <<std::dec<<std::endl; \
}
# define VAR(v)  { \
   std::cout<<#v<<" = "<<v<<std::endl; \
}
# define VARHEX(v)  { \
   std::cout<<#v<<" = 0x" \
            <<std::hex<<v \
            <<std::dec<<std::endl; \
}
# define VAR1(v) {std::cout << v << std::endl;}
# define VAR2(v1,v2)  { \
   std::cout<< "["<<#v1<<","<<#v2<<"] = " \
            << v1 << "," << v2 << std::endl; \
}
# define VAR3(v1,v2,v3)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<"]=" \
            <<v1<<","<<v2<<","<<v3<<std::endl; \
}
# define VAR4(v1,v2,v3,v4)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]=" \
            <<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl; \
}
# define VAR5(v1,v2,v3,v4,v5)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3 \
            <<","<<#v4<<","<<#v5<<"]=" \
            <<v1<<","<<v2<<","<<v3<<"," \
            <<v4<<","<<v5<<std::endl; \
}
# define VAR6(v1,v2,v3,v4,v5,v6)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<"," \
            <<#v4<<","<<#v5<<","<<#v6<<"]=" \
            <<v1<<","<<v2<<","<<v3<<"," \
            <<v4<<","<<v5<<","<<v6<<std::endl; \
}
# define VAR7(v1,v2,v3,v4,v5,v6,v7)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3 \
            <<","<<#v4<<","<<#v5<<","<<#v6 \
            <<","<<#v7<<"]="<<v1<<","<<v2<<","<<v3 \
            <<","<<v4<<","<<v5<<","<<v6<<","<<v7<<std::endl; \
}
# define VAR2V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<std::endl; \
}
# define VAR3V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<"," \
            <<v[1]<<","<<v[2]<<std::endl; \
}
# define VAR4V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<"," \
            <<v[1]<<","<<v[2]<<","<<v[3]<<std::endl; \
}
# define VAR5V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<"," \
            <<v[1]<<","<<v[2]<<","<<v[3]<<","<<v[4]<<std::endl; \
}
# define VARV(v)  { \
   int n = v.size(); \
   int w = (int)log10((double)n)+1; \
   for(int i=0;i<n;++i){ \
     if(!(i%6)) std::cout<<"\n["<<std::setw(w)<<i<<"]: "; \
     std::cout<<std::setw(10)<<v[i]<<" "; \
   } \
   std::cout<<std::endl; \
}
# define VARN(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ==="; \
   for(int i=0;i<n;++i){ \
     if(!(i%4)) std::cout<<"\n["<<std::setw(w)<<i<<"]: "; \
     std::cout<<std::setw(10)<<v[i]<<" "; \
   } \
   std::cout<<std::endl; \
}
# define VARNI(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ==="; \
   for(int i=0;i<n;++i){ \
     if(!(i%4)) std::cout<<"\n["<<std::setw(w)<<i<<"]: "; \
     std::cout<<std::setw(10)<<(int)v[i]<<" "; \
   } \
   std::cout<<std::endl; \
}
# define VARDUMP(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int i=0;i<n;++i){ \
     std::cout<<v[i]<<std::endl;; \
   } \
   std::cout<<std::endl; \
}
# define VARDUMPI(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int i=0;i<n;++i){ \
     std::cout<<(int)v[i]<<std::endl;; \
   } \
   std::cout<<std::endl; \
}
# define VAR4x4(m)  { \
  std::cout << "=== "<<#m<<" ==="<<std::endl; \
  for(int i=0; i<4; i++){ \
    for(int j=0; j< 4; j++){ \
      printf("  % 12.6f",m[i*4+j]); \
    } \
    printf("\n"); \
  } \
  printf("\n"); \
}
# define VARMxN(mat,m,n)  { \
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int i=0; i<m; i++){ \
    for(int j=0; j<n; j++){ \
      printf(" % 12.6f",mat[i*n+j]); \
    } \
    printf("\n"); \
  } \
  printf("\n"); \
}
# define VARMxN2(mat,m,n)  { \
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int i=0; i<m; i++){ \
    for(int j=0; j<n; j++){ \
      printf(" % 12.6f",mat[i][j]); \
    } \
    printf("\n"); \
  } \
  printf("\n"); \
}
# define DUMPHEX(P,N){ \
   printf("=== %s ===\n",#P); \
   unsigned char byte,*buf = (unsigned char*)(P); \
   int done=0,cpl=16,off=0,j=0; \
   while( ! done ){ \
	 if(off<N){ \
	   printf("[%6d]: ",off); \
	   for(j=0;j<cpl && ! done ;j++){ \
		if( off+j<N ){ \
		  memcpy(&byte,buf+off+j,sizeof(unsigned char)); \
		  printf("%02x ",byte); \
		} else done=1; \
	  } \
	  printf("\n"); \
	  off += j; \
	 } else done=1; \
   } \
}
# define DUMPBIN(P,N) { \
  printf("=== %s ===\n",#P); \
  unsigned char byte,mask,val,*buf = (unsigned char*)(P); \
  int done=0,cpl=4,off=0,j=0,bit; \
  while( ! done ){ \
	if( off<N ){ \
	  printf("[%6d]:",off); \
	  for(j=0;j<cpl && ! done ;j++){ \
		if( off+j<N ){ \
		  memcpy(&byte,buf+off+j,sizeof(unsigned char)); \
		  for(bit=7;bit>=0;--bit){ \
			mask = (unsigned char)(1<<bit); \
			val = byte & mask; \
			if( bit==7 || bit==3 ) \
			  printf(" "); \
			if(val==0) \
			  printf("0",val); \
			else \
			  printf("1"); \
		  } \
		} else done=1; \
	  } \
	  printf("\n"); \
	  off += j; \
	} else done=1; \
  } \
}
# define PERROR(m) { \
  std::cerr<<"==========================="<<std::endl; \
  std::cerr<<"ERR: "; \
  perror(m); \
  std::cerr<<"ERR: File:"<<__FILE__<<std::endl; \
  std::cerr<<"ERR: Line:"<<__LINE__<<std::endl; \
  std::cerr<<"==========================="<<std::endl; \
}
# define ERROR(m) { \
  std::cerr<<"==========================="<<std::endl; \
  std::cerr<<"ERR: " << m << std::endl; \
  std::cerr<<"ERR: File:"<<__FILE__<<std::endl; \
  std::cerr<<"ERR: Func:"<<__FUNCTION__<<std::endl; \
  std::cerr<<"ERR: Line:"<<__LINE__<<std::endl; \
  std::cerr<<"==========================="<<std::endl; \
}


// aliases

#else
# define FANCYMESG(m)
# define FANCYVAR(v)
# define FANCYVAR2(v1,v2)
# define FANCYVAR3(v1,v2,v3)
# define FANCYVAR4(v1,v2,v3,v4)
# define MESG(m) 
# define VAR(v)  
# define VARHEX(v)  
# define VAR1(v1)
# define VAR2(v1,v2)
# define VAR3(v1,v2,v3)
# define VAR4(v1,v2,v3,v4)
# define VAR5(v1,v2,v3,v4,v5)
# define VAR6(v1,v2,v3,v4,v5,v6)
# define VAR7(v1,v2,v3,v4,v5,v6,v7)
# define VAR2V(v)
# define VAR3V(v)
# define VAR4V(v)
# define VAR5V(v)
# define VARV(v)
# define MESGVAR(m,v) 
# define MESGVAR2(m,v1,v2) 
# define MESGVAR3(m,v1,v2,v3) 
# define MESGVAR4(m,v1,v2,v3,v4) 
# define MESGVARHEX(m,v)
# define VARN(v,n)
# define VARNI(v,n)
# define VARDUMP(v,n)
# define VARDUMPI(v,n)
# define VAR4x4(mat)
# define VARMxN(mat,m,n) 
# define VARMxN2(mat,m,n)
# define PERROR(m)
# define ERROR(m)
#endif

# define _FANCYMESG(m) { \
   std::cout<<"===== "<<m<<" ====="<<std::endl; \
}
# define _FANCYVAR(v) { \
   std::cout<<"===== "<<#v<<" = "<<v<<" ====="<<std::endl; \
}
# define _FANCYVAR2(v1,v2) { \
   std::cout<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"====="<<std::endl; \
}
# define _FANCYVAR3(v1,v2,v3) { \
   std::cout<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"====="<<std::endl; \
}
# define _FANCYVAR4(v1,v2,v3,v4) { \
   std::cout<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"," \
            <<#v4<<"="<<v4<<"====="<<std::endl; \
}
# define _MESG(m) { \
   std::cout << m << std::endl; \
}
# define _MESGVAR(m,v) { \
   std::cout<<m<<":"<<#v<<" = "<<v<<std::endl; \
}
# define _MESGVAR2(m,v1,v2) { \
   std::cout<<m<<":["<<#v1<<","<<#v2<<"]=" \
            <<v1<<","<<v2<<std::endl; \
}
# define _MESGVAR3(m,v1,v2,v3) { \
   std::cout<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<"]=" \
           <<v1<<","<<v2<<","<<v3<<std::endl; \
}
# define _MESGVAR4(m,v1,v2,v3,v4) { \
   std::cout<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]=" \
           <<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl; \
}
# define _MESGVARHEX(m,v) { \
   std::cout<<m<<":"<<#v<<" = 0x" \
            <<std::hex<<(void*)v \
            <<std::dec<<std::endl; \
}
# define _VAR(v)  { \
   std::cout<<#v<<" = "<<v<<std::endl; \
}
# define _VARHEX(v)  { \
   std::cout<<#v<<" = 0x" \
            <<std::hex<<v \
            <<std::dec<<std::endl; \
}
# define _VAR1(v) {std::cout << v << std::endl;}
# define _VAR2(v1,v2)  { \
   std::cout<< "["<<#v1<<","<<#v2<<"] = " \
            << v1 << "," << v2 << std::endl; \
}
# define _VAR3(v1,v2,v3)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<"]=" \
            <<v1<<","<<v2<<","<<v3<<std::endl; \
}
# define _VAR4(v1,v2,v3,v4)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]=" \
            <<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl; \
}
# define _VAR5(v1,v2,v3,v4,v5)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3 \
            <<","<<#v4<<","<<#v5<<"]=" \
            <<v1<<","<<v2<<","<<v3<<"," \
            <<v4<<","<<v5<<std::endl; \
}
# define _VAR6(v1,v2,v3,v4,v5,v6)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<"," \
            <<#v4<<","<<#v5<<","<<#v6<<"]=" \
            <<v1<<","<<v2<<","<<v3<<"," \
            <<v4<<","<<v5<<","<<v6<<std::endl; \
}
# define _VAR7(v1,v2,v3,v4,v5,v6,v7)  { \
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3 \
            <<","<<#v4<<","<<#v5<<","<<#v6 \
            <<","<<#v7<<"]="<<v1<<","<<v2<<","<<v3 \
            <<","<<v4<<","<<v5<<","<<v6<<","<<v7<<std::endl; \
}
# define _VAR2V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<std::endl; \
}
# define _VAR3V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<"," \
            <<v[1]<<","<<v[2]<<std::endl; \
}
# define _VAR4V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<"," \
            <<v[1]<<","<<v[2]<<","<<v[3]<<std::endl; \
}
# define _VAR5V(v)  { \
   std::cout<<"["<<#v<<"]="<<v[0]<<"," \
            <<v[1]<<","<<v[2]<<","<<v[3]<<","<<v[4]<<std::endl; \
}
# define _VARV(v)  { \
   int n = v.size(); \
   int w = (int)log10((double)n)+1; \
   for(int i=0;i<n;++i){ \
     if(!(i%4)) std::cout<<"\n["<<std::setw(w)<<i<<"]: "; \
     std::cout<<std::setw(10)<<v[i]<<" "; \
   } \
   std::cout<<std::endl; \
}
# define _VARN(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ==="; \
   for(int i=0;i<n;++i){ \
     if(!(i%4)) std::cout<<"\n["<<std::setw(w)<<i<<"]: "; \
     std::cout<<std::setw(10)<<v[i]<<" "; \
   } \
   std::cout<<std::endl; \
}
# define _VARNI(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ==="; \
   for(int i=0;i<n;++i){ \
     if(!(i%4)) std::cout<<"\n["<<std::setw(w)<<i<<"]: "; \
     std::cout<<std::setw(10)<<(int)v[i]<<" "; \
   } \
   std::cout<<std::endl; \
}
# define _VARDUMP(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int i=0;i<n;++i){ \
     std::cout<<v[i]<<std::endl;; \
   } \
   std::cout<<std::endl; \
}
#define _VARDUMPI(v,n)  { \
   int w = (int)log10((double)n)+1; \
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int i=0;i<n;++i){ \
     std::cout<<(int)v[i]<<std::endl;; \
   } \
   std::cout<<std::endl; \
}
# define _VAR4x4(m)  { \
  std::cout << "=== "<<#m<<" ==="<<std::endl; \
  for(int i=0; i<4; i++){ \
    for(int j=0; j< 4; j++){ \
      printf("  % 12.6f",m[i*4+j]); \
    } \
    printf("\n"); \
  } \
  printf("\n"); \
}
# define _VARMxN(mat,m,n)  { \
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int i=0; i<m; i++){ \
    for(int j=0; j<n; j++){ \
      printf(" % 12.6f",mat[i*n+j]); \
    } \
    printf("\n"); \
  } \
  printf("\n"); \
}
# define _VARMxN2(mat,m,n)  { \
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int i=0; i<m; i++){ \
    for(int j=0; j<n; j++){ \
      printf(" % 12.6f",mat[i][j]); \
    } \
    printf("\n"); \
  } \
  printf("\n"); \
}
# define _DUMPHEX(P,N){ \
   printf("=== %s ===\n",#P); \
   unsigned char byte,*buf = (unsigned char*)(P); \
   int done=0,cpl=16,off=0,j=0; \
   while( ! done ){ \
	 if(off<N){ \
	   printf("[%6d]: ",off); \
	   for(j=0;j<cpl && ! done ;j++){ \
		if( off+j<N ){ \
		  memcpy(&byte,buf+off+j,sizeof(unsigned char)); \
		  printf("%02x ",byte); \
		} else done=1; \
	  } \
	  printf("\n"); \
	  off += j; \
	 } else done=1; \
   } \
}
# define _DUMPBIN(P,N) { \
  printf("=== %s ===\n",#P); \
  unsigned char byte,mask,val,*buf = (unsigned char*)(P); \
  int done=0,cpl=4,off=0,j=0,bit; \
  while( ! done ){ \
	if( off<N ){ \
	  printf("[%6d]:",off); \
	  for(j=0;j<cpl && ! done ;j++){ \
		if( off+j<N ){ \
		  memcpy(&byte,buf+off+j,sizeof(unsigned char)); \
		  for(bit=7;bit>=0;--bit){ \
			mask = (unsigned char)(1<<bit); \
			val = byte & mask; \
			if( bit==7 || bit==3 ) \
			  printf(" "); \
			if(val==0) \
			  printf("0",val); \
			else \
			  printf("1"); \
		  } \
		} else done=1; \
	  } \
	  printf("\n"); \
	  off += j; \
	} else done=1; \
  } \
}
# define _PERROR(m) { \
  std::cerr<<"==========================="<<std::endl; \
  std::cerr<<"ERR: "; \
  perror(m); \
  std::cerr<<"ERR: File:"<<__FILE__<<std::endl; \
  std::cerr<<"ERR: Line:"<<__LINE__<<std::endl; \
  std::cerr<<"==========================="<<std::endl; \
}
# define _ERROR(m) { \
  std::cerr<<"==========================="<<std::endl; \
  std::cerr<<"ERR: " << m << std::endl; \
  std::cerr<<"ERR: File:"<<__FILE__<<std::endl; \
  std::cerr<<"ERR: Func:"<<__FUNCTION__<<std::endl; \
  std::cerr<<"ERR: Line:"<<__LINE__<<std::endl; \
  std::cerr<<"==========================="<<std::endl; \
}

#endif
