##############################################################################
# GLOBAL COMPILER FLAGS
##############################################################################

include ../Global.defs

##############################################################################
# LOCAL COMPILER FLAGS
##############################################################################

LOCAL_FLAGS = -I. -I../ 

##############################################################################
# LIBRARY DEFINES
##############################################################################

LIB_O  = Orientable.o Arrow.o Sphere.o
LIB_H  = Obj.h Orientable.h Arrow.h Sphere.h
LIB_C  = $(LIB_O:.o=.C) 
RCS_FILES = Makefile $(LIB_H) $(LIB_C) 
RCS_DIRS  = 

default: $(LIB_O)

##############################################################################
# Standard rules: clean & install
##############################################################################

c: clean
clean: 
	/bin/rm -fr so_locations ii_files *.o *~ *.bak
	/bin/rm -fr `file * | grep ELF | awk -F: ' {print $$1}'`


