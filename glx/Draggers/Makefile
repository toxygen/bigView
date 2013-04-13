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

LIB_O  = Dragger.o Dragger2D.o Dragger3D.o \
	Palette.o WorldPalette.o Slider.o Multislider.o Histogram.o \
	Crosshair2D.o TFeditor.o Colorbar.o Plotter.o \
	Vector.o SphereDragger.o SV.o Card.o Crosshair3D.o Check.o \
	SelectablePlot2D.o

LIB_H  = $(LIB_O:.o=.h) 
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

install:

