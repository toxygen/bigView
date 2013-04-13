//////////////////////////////////////////////////////////////////////////
//////////////////////////////// glerr.C /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include "glerr.h"
#include "debug.h"

using namespace std;

GLenum 
checkError(string call)
{ 
  GLenum errVar = glGetError();
  switch( errVar){ 
    case GL_NO_ERROR: 
      break; 
    case GL_INVALID_ENUM: 
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "GL_INVALID_ENUM" << std::endl; 
      break; 
    case GL_INVALID_VALUE:
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "GL_INVALID_VALUE" << std::endl; 
      break; 
    case GL_INVALID_OPERATION: 
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "GL_INVALID_OPERATION" << std::endl; 
      break;
    case GL_STACK_OVERFLOW: 
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "GL_STACK_OVERFLOW"<< std::endl; 
      break;
    case GL_STACK_UNDERFLOW:
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "GL_STACK_UNDERFLOW" << std::endl; 
      break;
    case GL_OUT_OF_MEMORY:
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "GL_OUT_OF_MEMORY" << std::endl; 
      break; 
    default:
      if( call.length() ) std::cerr << call << ": ";
      std::cerr << "Unknown OpenGL error : "<<errVar<<std::endl; 
      break; 
  } 
  std::cerr<<std::flush; 
  return errVar;
}
