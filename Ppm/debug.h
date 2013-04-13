#ifndef VT_DEBUG_H
#define VT_DEBUG_H

#include <iostream>    // for cout, endl
#include <iomanip>

#ifdef DEBUG 

# define FANCYMESG(m) {\
   std::cout << "===== " << m << " =====" << std::endl;\
}

# define MESG(m) {\
   std::cout << m << std::endl;\
}
# define MESGVAR(m,v) {\
   std::cout << m << ":" << #v << " = " << v << std::endl;\
}
# define MESGVAR2(m,v1,v2) {\
   std::cout<<m<<":["<<#v1<<","<<#v2<<"]="<<v1<<","<<v2<<std::endl;\
}

# define MESGVAR3(m,v1,v2,v3) {\
   std::cout<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
}
# define VAR(v)  {\
   std::cout << #v << " = " << v << std::endl;\
}
# define VARHEX(v)  {\
   std::cout << #v << " = 0x"<<std::hex<< (void*)v << std::dec << std::endl;\
}

// no frills
# define VAR1(v)  {\
   std::cout << v << std::endl;\
}
# define VAR2(v1,v2)  {\
   std::cout << "["<<#v1<<","<<#v2<<"] = " << v1 << "," << v2 << std::endl;\
}
# define VAR3(v1,v2,v3)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
}
# define VAR4(v1,v2,v3,v4)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl;\
}
# define VAR5(v1,v2,v3,v4,v5)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<std::endl;\
}
# define VAR6(v1,v2,v3,v4,v5,v6)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<","<<#v6<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<","<<v6<<std::endl;\
}

# define VAR2V(v)  {\
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<std::endl;\
}
# define VAR3V(v)  {\
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<std::endl;\
}
# define VAR4V(v)  {\
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<","<<v[3]<<std::endl;\
}
# define VARV(v)  {\
   int _n_ = v.size(); \
   int _w_ = (int)log10((double)_n_)+1;\
   for(int _i_=0;_i_<_n_;++_i_){ \
     if( ! (_i_ % 6) ) std::cout << "\n["<< std::setw(_w_) <<_i_ << "]: "; \
     std::cout<<std::setw(10)<<v[_i_]<<" ";\
   } \
   std::cout<<std::endl;\
}

# define VARN(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ==="; \
   for(int _i_=0;_i_<n;++_i_){ \
     if( ! (_i_ % 6) ) std::cout << "\n["<< std::setw(_w_) <<_i_ << "]: "; \
     std::cout<<std::setw(10)<<v[_i_]<<" ";\
   } \
   std::cout<<std::endl;\
}
# define VARNI(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ==="; \
   for(int _i_=0;_i_<n;++_i_){ \
     if( ! (_i_ % 6) ) std::cout << "\n["<< std::setw(_w_) << _i_ << "]: "; \
     std::cout<<std::setw(10)<<(int)v[_i_]<<" ";\
   } \
   std::cout<<std::endl;\
}
# define VARDUMP(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<v[_i_]<<std::endl;;\
   } \
   std::cout<<std::endl;\
}
#define VARDUMPI(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<(int)v[_i_]<<std::endl;;\
   } \
   std::cout<<std::endl;\
}
# define VAR4x4(m)  {\
  std::cout << "=== "<<#m<<" ==="<<std::endl; \
  for(int _i_=0; _i_<4; _i_++){\
    for(int _j_=0; _j_< 4; _j_++){\
      printf("  % 12.6f",m[_i_*4+_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
}

# define VARMxN(mat,m,n)  {\
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_*n+_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
}

# define VARMxN2(mat,m,n)  {\
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_][_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
}

#define PERROR(m) {\
  std::cerr<<"ERR: ";\
  perror(m);\
  std::cerr<<"ERR: File:"<<__FILE__<<std::endl;\
  std::cerr<<"ERR: Line:"<<__LINE__<<std::endl;\
}


// aliases

#else
# define FANCYMESG(m)
# define MESG(m) 
# define VAR(v)  
# define VARHEX(v)  
# define VAR1(v1)
# define VAR2(v1,v2)
# define VAR3(v1,v2,v3)
# define VAR4(v1,v2,v3,v4)
# define VAR5(v1,v2,v3,v4,v5)
# define VAR6(v1,v2,v3,v4,v5,v6)
# define VAR2V(v)
# define VAR3V(v)
# define VAR4V(v)
# define VARV(v)
# define MESGVAR(m,v) 
# define MESGVAR2(m,v1,v2) 
# define MESGVAR3(m,v1,v2,v3) 
# define VARN(v,n)
# define VARNI(v,n)
# define VARDUMP(v,n)
# define VARDUMPI(v,n)
# define VAR4x4(mat)
# define VARMxN(mat,m,n) 
# define VARMxN2(mat,m,n)
# define PERROR(m)
#endif

#define _FANCYMESG(m) {\
   std::cout << "===== " << m << " =====" << std::endl;\
}

#define _MESG(m) {\
   std::cout << m << std::endl;\
}
#define _MESGVAR(m,v) {\
   std::cout << m << ":" << #v << " = " << v << std::endl;\
}
#define _MESGVAR2(m,v1,v2)  {\
   std::cout<<m<<":["<<#v1<<","<<#v2<<"] = " << v1 << "," << v2 << std::endl;\
}
# define _MESGVAR3(m,v1,v2,v3) {\
   std::cout<<m<<":["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
}
#define _VAR(v)  {\
   std::cout << #v << " = " << v << std::endl;\
}
#define _VARHEX(v)  {\
   std::cout << #v << " = 0x" <<std::hex<< (void*)v << std::dec << std::endl;\
}

// no frills
#define _VAR1(v)  {\
   std::cout << v << std::endl;\
}
#define _VAR2(v1,v2)  {\
   std::cout << "["<<#v1<<","<<#v2<<"] = " << v1 << "," << v2 << std::endl;\
}
#define _VAR3(v1,v2,v3)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<"]="<<v1<<","<<v2<<","<<v3<<std::endl;\
}
#define _VAR4(v1,v2,v3,v4)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<std::endl;\
}
#define _VAR5(v1,v2,v3,v4,v5)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<std::endl;\
}
#define _VAR6(v1,v2,v3,v4,v5,v6)  {\
   std::cout<<"["<<#v1<<","<<#v2<<","<<#v3<<","<<#v4<<","<<#v5<<","<<#v6<<"]="<<v1<<","<<v2<<","<<v3<<","<<v4<<","<<v5<<","<<v6<<std::endl;\
}

#define _VAR2V(v)  {\
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<std::endl;\
}
#define _VAR3V(v)  {\
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<std::endl;\
}
#define _VAR4V(v)  {\
   std::cout<<"["<<#v<<"]="<<v[0]<<","<<v[1]<<","<<v[2]<<","<<v[3]<<std::endl;\
}

# define _VARV(v)  {\
   int _n_ = v.size(); \
   int _w_ = (int)log10((double)_n_)+1;\
   for(int _i_=0;_i_<_n_;++_i_){ \
     if( ! (_i_ % 6) ) std::cout << "\n["<< std::setw(_w_) <<_i_ << "]: "; \
     std::cout<<std::setw(10)<<v[_i_]<<" ";\
   } \
   std::cout<<std::endl;\
}

#define _VARN(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ==="; \
   for(int _i_=0;_i_<n;++_i_){ \
     if( ! (_i_ % 6) ) std::cout << "\n["<< std::setw(_w_) << _i_ << "]: "; \
     std::cout<<std::setw(10)<<v[_i_]<<" ";\
   } \
   std::cout<<std::endl;\
}
#define _VARNI(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ==="; \
   for(int _i_=0;_i_<n;++_i_){ \
     if( ! (_i_ % 6) ) std::cout << "\n["<< std::setw(_w_) << _i_ << "]: "; \
     std::cout<<std::setw(10)<<(int)v[_i_]<<" ";\
   } \
   std::cout<<std::endl;\
}
#define _VARDUMP(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<v[_i_]<<std::endl;;\
   } \
   std::cout<<std::endl;\
}
#define _VARDUMPI(v,n)  {\
   int _w_ = (int)log10((double)n)+1;\
   std::cout << "=== "<<#v<<" ===" << std::endl; \
   for(int _i_=0;_i_<n;++_i_){ \
     std::cout<<(int)v[_i_]<<std::endl;;\
   } \
   std::cout<<std::endl;\
}

#define _VAR4x4(m)  {\
  std::cout << "=== "<<#m<<" ==="<<std::endl; \
  for(int _i_=0; _i_<4; _i_++){\
    for(int _j_=0; _j_< 4; _j_++){\
      printf("  % 12.6f",m[_i_*4+_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
}

#define _VARMxN(mat,m,n)  {\
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_*n+_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
}
#define _VARMxN2(mat,m,n)  {\
  std::cout << "=== "<<#mat<<" ==="<<std::endl; \
  for(int _i_=0; _i_<m; _i_++){\
    for(int _j_=0; _j_<n; _j_++){\
      printf(" % 12.6f",mat[_i_][_j_]);\
    }\
    printf("\n");\
  }\
  printf("\n");\
}

#define _PERROR(m) {\
  std::cerr<<"ERR: ";\
  perror(m);\
  std::cerr<<"ERR: File:"<<__FILE__<<std::endl;\
  std::cerr<<"ERR: Line:"<<__LINE__<<std::endl;\
}

#endif
