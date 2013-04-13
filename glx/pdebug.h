#ifndef VT_DEBUG_H
#define VT_DEBUG_H

#include <iostream>    // for cout, endl
#include <iomanip>
#include <errno.h>

extern void plock(void);
extern void punlock(void);
extern std::string _ptid_(void);

#ifdef DEBUG 

# define FANCYMESG(m) {\
   plock(); \
   std::cout<<_ptid_() << "===== " << m << " =====" << std::endl;\
   punlock(); \
}

# define FANCYVAR(v) {\
   plock(); \
   std::cout<<_ptid_() << "===== " << #v << " = " << v << " =====" << std::endl;\
   punlock(); \
}
# define FANCYVAR2(v1,v2) { \
   plock(); \
   std::cout<<_ptid_()<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"====="<<std::endl; \
   punlock(); \
}
# define FANCYVAR3(v1,v2,v3) { \
   plock(); \
   std::cout<<_ptid_()<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"====="<<std::endl; \
   punlock(); \
}
# define FANCYVAR4(v1,v2,v3,v4) { \
   plock(); \
   std::cout<<_ptid_()<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"," \
            <<#v4<<"="<<v4<<"====="<<std::endl; \
   punlock(); \
}
# define MESG(m) {\
   plock(); \
   std::cout<<_ptid_() << m << std::endl;\
   punlock(); \
}
# define MESGVAR(m,v) {\
   plock(); \
   std::cout<<_ptid_() << m << ":" << #v << " = " << v << std::endl;\
   punlock(); \
}
# define MESGVAR2(m,v1,v2) {\
   plock(); \
   std::cout<<_ptid_()<<m<<":["<<#v1<<","<<#v2<<"]="<<v1<<","<<v2<<std::endl;\
   punlock(); \
}
# define MESGVAR3(m,v1,v2,v3) {\
   plock(); \
   std::cout<<_ptid_()<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
   punlock(); \
}
# define MESGVAR4(m,v1,v2,v3,v4) { \
   plock(); \
   std::cout<<_ptid_() <<m<<":["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]=" \
           <<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl; \
   punlock(); \
}
# define MESGVARHEX(m,v) {\
   plock(); \
   std::cout<<_ptid_()<<m<<":"<<#v<<" = 0x"<<std::hex<<(void*)v<<std::dec<<std::endl;\
   punlock(); \
}

# define VAR(v)  {\
   plock(); \
   std::cout<<_ptid_() << #v << " = " << v << std::endl;\
   punlock(); \
}
# define VARHEX(v)  {\
   plock(); \
   std::cout<<_ptid_() << #v << " = 0x"<<std::hex<< (void*)v << std::dec << std::endl;\
   punlock(); \
}

// no frills
# define VAR1(v)  {\
   plock(); \
   std::cout<<_ptid_() << v << std::endl;\
   punlock(); \
}
# define VAR2(v1,v2)  {\
   plock(); \
   std::cout<<_ptid_() << "["<<#v1<<","<<#v2<<"] = " << v1 << "," << v2 << std::endl;\
   punlock(); \
}
# define VAR3(v1,v2,v3)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
   punlock(); \
}
# define VAR4(v1,v2,v3,v4)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl;\
   punlock(); \
}
# define VAR5(v1,v2,v3,v4,v5)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<std::endl;\
   punlock(); \
}
# define VAR6(v1,v2,v3,v4,v5,v6)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<","<<#v6<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<","<<v6<<std::endl;\
   punlock(); \
}
# define VAR7(v1,v2,v3,v4,v5,v6,v7)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<","<<#v6<<","<<#v7<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<","<<v6<<","<<v7<<std::endl;\
   punlock(); \
}

# define VAR2V(v)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<std::endl;\
   punlock(); \
}
# define VAR3V(v)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<std::endl;\
   punlock(); \
}
# define VAR4V(v)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<","<<v[3]<<std::endl;\
   punlock(); \
}

# define VARV(v)  {\
   plock(); \
   int _n_ = v.size(); \
   int _w_ = (int)log10((double)_n_)+1;\
   for(int _i_=0;_i_<_n_;++_i_){ \
     if( ! (_i_ % 6) ) std::cout<<_ptid_() << "\n["<< std::setw(_w_) <<_i_ << "]: "; \
     std::cout<<_ptid_()<<std::setw(10)<<v[_i_]<<" ";\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

# define VARN(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ==="; \
   for(int i=0;i<n;++i){ \
     if( ! (i % 6) ) std::cout<<_ptid_() << "\n["<< std::setw(_w_) << i << "]: "; \
     std::cout<<_ptid_()<<std::setw(10)<<v[i]<<" ";\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}
# define VARNI(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ==="; \
   for(int _i_=0;_i_<n;++_i_){ \
     if( ! (_i_ % 6) ) std::cout<<_ptid_() << "\n["<< std::setw(_w_) << _i_ << "]: "; \
     std::cout<<_ptid_()<<std::setw(10)<<(int)v[_i_]<<" ";\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

# define VARDUMP(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<_ptid_()<<v[_i_]<<std::endl;;\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}
#define VARDUMPI(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<_ptid_()<<(int)v[_i_]<<std::endl;;\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

# define VAR4x4(m)  {\
   plock(); \
   std::cout<<_ptid_() << "=== "<<#m<<" ==="<<std::endl; \
   for(int _i_=0; _i_<4; _i_++){\
     for(int _j_=0; _j_< 4; _j_++){\
       printf("  % 12.6f",m[_i_*4+_j_]);\
     }\
     printf("\n");\
   }\
   printf("\n");\
   punlock(); \
}

# define VARMxN(mat,m,n)  {\
  plock(); \
  std::cout<<_ptid_() << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_*n+_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
  punlock(); \
}

# define VARMxN2(mat,m,n)  {\
  plock(); \
  std::cout<<_ptid_() << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_][_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
  punlock(); \
}

#define PERROR(m) {\
  plock(); \
  std::cerr<<"==========================="<<std::endl;\
  std::cerr<<"ERR: ";\
  perror(m);\
  std::cerr<<"ERR: errno: "<<errno<<std::endl;\
  std::cerr<<"ERR:  File: "<<__FILE__<<std::endl;\
  std::cerr<<"ERR:  Line: "<<__LINE__<<std::endl;\
  std::cerr<<"==========================="<<std::endl;\
  punlock(); \
}
#define ERROR(m) {\
  plock(); \
  std::cerr<<"==========================="<<std::endl;\
  std::cerr<<"ERR: " << m << std::endl;\
  std::cerr<<"ERR: File: "<<__FILE__<<std::endl;\
  std::cerr<<"ERR: Func: "<<__FUNCTION__<<std::endl;\
  std::cerr<<"ERR: Line: "<<__LINE__<<std::endl;\
  std::cerr<<"==========================="<<std::endl;\
  punlock(); \
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
# define MESGVAR(m,v) 
# define MESGVAR2(m,v1,v2) 
# define MESGVAR3(m,v1,v2,v3) 
# define MESGVAR4(m,v1,v2,v3,v4) 
# define MESGVARHEX(m,v)
# define VARN(v,n)
# define VARNI(v,n)
# define VARDUMP(v,n)
# define VARDUMPI(v,n)
# define VAR4x4(m)
# define VARMxN(mat,m,n) 
# define VARMxN2(mat,m,n)
# define PERROR(m)
# define ERROR(m)
#endif

#define _FANCYMESG(m) {\
   plock(); \
   std::cout<<_ptid_() << "===== " << m << " =====" << std::endl;\
   punlock(); \
}

#define _FANCYVAR(v) {\
   plock(); \
   std::cout<<_ptid_() << "===== " << #v << " = " << v << " =====" << std::endl;\
   punlock(); \
}
# define _FANCYVAR2(v1,v2) { \
   plock(); \
   std::cout<<_ptid_()<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"====="<<std::endl; \
   punlock(); \
}
# define _FANCYVAR3(v1,v2,v3) { \
   plock(); \
   std::cout<<_ptid_()<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"====="<<std::endl; \
   punlock(); \
}
# define _FANCYVAR4(v1,v2,v3,v4) { \
   plock(); \
   std::cout<<_ptid_()<<"====="<<#v1<<"="<<v1<<"," \
            <<#v2<<"="<<v2<<"," \
            <<#v3<<"="<<v3<<"," \
            <<#v4<<"="<<v4<<"====="<<std::endl; \
   punlock(); \
}
#define _MESG(m) {\
   plock(); \
   std::cout<<_ptid_() << m << std::endl;\
   punlock(); \
}
#define _MESGVAR(m,v) {\
   plock(); \
   std::cout<<_ptid_() << m << ":" << #v << " = " << v << std::endl;\
   punlock(); \
}
#define _MESGVAR2(m,v1,v2)  {\
   plock(); \
   std::cout<<_ptid_()<<m<<":["<<#v1<<","<<#v2<<"] = " << v1 << "," << v2 << std::endl;\
   punlock(); \
}
# define _MESGVAR3(m,v1,v2,v3) {\
   plock(); \
   std::cout<<_ptid_()<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
   punlock(); \
}
# define _MESGVAR4(m,v1,v2,v3,v4) { \
   plock(); \
   std::cout<<_ptid_() <<m<<":["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]=" \
           <<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl; \
   punlock(); \
}
# define _MESGVARHEX(m,v) {\
   plock(); \
   std::cout<<_ptid_()<<m<<":"<<#v<<" = 0x"<<std::hex<<(void*)v<<std::dec<<std::endl;\
   punlock(); \
}

#define _VAR(v)  {\
   plock(); \
   std::cout<<_ptid_() << #v << " = " << v << std::endl;\
   punlock(); \
}
# define _VARHEX(v)  {\
   plock(); \
   std::cout<<_ptid_() << #v << " = 0x" <<std::hex<< (void*)v << std::dec << std::endl;\
   punlock(); \
}

// no frills
# define _VAR1(v)  {\
   plock(); \
   std::cout<<_ptid_() << v << std::endl;\
   punlock(); \
}
# define _VAR2(v1,v2)  {\
   plock(); \
   std::cout<<_ptid_() << "["<<#v1<<","<<#v2<<"] = " << v1 << "," << v2 << std::endl;\
   punlock(); \
}
# define _VAR3(v1,v2,v3)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
   punlock(); \
}
# define _VAR4(v1,v2,v3,v4)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl;\
   punlock(); \
}
#define _VAR5(v1,v2,v3,v4,v5)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<std::endl;\
   punlock(); \
}
#define _VAR6(v1,v2,v3,v4,v5,v6)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<","<<#v6<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<","<<v6<<std::endl;\
   punlock(); \
}
#define _VAR7(v1,v2,v3,v4,v5,v6,v7)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<","<<#v6<<","<<#v7<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<","<<v6<<","<<v7<<std::endl;\
   punlock(); \
}

# define _VAR2V(v)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<std::endl;\
   punlock(); \
}
# define _VAR3V(v)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<std::endl;\
   punlock(); \
}
# define _VAR4V(v)  {\
   plock(); \
   std::cout<<_ptid_()<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<","<<v[3]<<std::endl;\
   punlock(); \
}

# define _VARV(v)  {\
   plock(); \
   int _n_ = v.size(); \
   int _w_ = (int)log10((double)_n_)+1;\
   for(int _i_=0;_i_<_n_;++_i_){ \
     if( ! (_i_ % 6) ) std::cout<<_ptid_() << "\n["<< std::setw(_w_) <<_i_ << "]: "; \
     std::cout<<_ptid_()<<std::setw(10)<<v[_i_]<<" ";\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

# define _VARN(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ==="; \
   for(int i=0;i<n;++i){ \
     if( ! (i % 6) ) std::cout<<_ptid_() << "\n["<< std::setw(_w_) << i << "]: "; \
     std::cout<<_ptid_()<<std::setw(10)<<v[i]<<" ";\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

#define _VARNI(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ==="; \
   for(int _i_=0;_i_<n;++_i_){ \
     if( ! (_i_ % 6) ) std::cout<<_ptid_() << "\n["<< std::setw(_w_) << _i_ << "]: "; \
     std::cout<<_ptid_()<<std::setw(10)<<(int)v[_i_]<<" ";\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

#define _VARDUMP(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<_ptid_()<<v[_i_]<<std::endl;;\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}
#define _VARDUMPI(v,n)  {\
   plock(); \
   int _w_ = (int)log10((double)n)+1;\
   std::cout<<_ptid_() << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<_ptid_()<<(int)v[_i_]<<std::endl;;\
   } \
   std::cout<<_ptid_()<<std::endl;\
   punlock(); \
}

# define _VAR4x4(m)  {\
  plock(); \
  std::cout<<_ptid_() << "=== "<<#m<<" ==="<<std::endl; \
  for(int i=0; i<4; i++){\
    for(int j=0; j< 4; j++){\
      printf("  % 8.5f",m[i*4+j]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
  punlock();\
}
#define _VARMxN(mat,m,n)  {\
  plock(); \
  std::cout<<_ptid_() << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_*n+_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
  punlock();\
}
#define _VARMxN2(mat,m,n)  {\
  plock(); \
  std::cout<<_ptid_() << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_][_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
  punlock();\
}

#define _PERROR(m) {\
  plock(); \
  std::cerr<<"ERR: ";\
  perror(m);\
  std::cerr<<"ERR: errno: "<<errno<<std::endl;\
  std::cerr<<"ERR:  File: "<<__FILE__<<std::endl;\
  std::cerr<<"ERR:  Line: "<<__LINE__<<std::endl;\
  punlock();\
}

#define _ERROR(m) {\
  plock(); \
  std::cerr<<"==========================="<<std::endl;\
  std::cerr<<"ERR: " << m << std::endl;\
  std::cerr<<"ERR: File: "<<__FILE__<<std::endl;\
  std::cerr<<"ERR: Func: "<<__FUNCTION__<<std::endl;\
  std::cerr<<"ERR: Line: "<<__LINE__<<std::endl;\
  std::cerr<<"==========================="<<std::endl;\
  punlock();\
}

#endif
