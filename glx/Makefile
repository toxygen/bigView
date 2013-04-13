
include Global.defs

#PROF = -pg
PROF =

LOCAL_FLAGS = $(PROF) -LANG:std -DOPEN_GL -I.
LINK = $(GLBL_FLAGS) $(PROF) -L. -lGlx\
	-lGLw -lGL -lXm -lXt -lXext -lX11 -lm $(GLBL_LINK) -lpthread

LIB   = libGlx.a
LIB_O = GLX.o glxTrackball.o glxTrackpad.o glxQuat.o glxVector.o \
	glxColormap.o glxFFont.o \
	Draggers/Dragger.o Draggers/Dragger2D.o Draggers/Dragger3D.o \
	Draggers/Palette.o Draggers/WorldPalette.o Draggers/Histogram.o \
	Draggers/Slider.o Draggers/Multislider.o \
	Draggers/Crosshair2D.o Draggers/Crosshair3D.o \
	Draggers/TFeditor.o Draggers/Colorbar.o Draggers/Plotter.o \
	Draggers/Vector.o Draggers/SphereDragger.o \
	Draggers/SV.o Draggers/Card.o Draggers/Check.o \
	Draggers/SelectablePlot2D.o \
	Objects/Orientable.o Objects/Arrow.o Objects/Sphere.o  \
	Colorpicker.o str.o pdebug.o
LIB_C = $(LIB_O:.o=.C)
LIB_H = $(LIB_C:.C=.h)
APP   = pick

default: $(LIB) $(APP) 

$(LIB): subLibs $(LIB_O) $(LIB_H)
	-@ $(CPLUSPLUS) $(GLBL_FLAGS) $(LOCAL_FLAGS) $(LIB_O) 2> /dev/null
	$(AR) $(ARFLAGS) $(LIB) $(LIB_O) 
	$(RANLIB) $(LIB)

subLibs:
	cd Draggers; $(MAKE) 
	cd Objects; $(MAKE) 

examples:
	cd Examples; $(MAKE) 

pick: $(LIB) pick.o dumpstate.o
	$(CPLUSPLUS) -o pick pick.o dumpstate.o $(LINK)

work: $(LIB) work.o
	$(CPLUSPLUS) -o work work.o $(LINK)

##############################################################################
# Standard rules: clean, tag, localDepend
##############################################################################

SRCS = -name '*.[cCfFhH]' -o -name '*.cxx' -o -name Makefile -o -name '*.defs'
tar:
	cwd=`pwd | xargs basename`;\
	cd ../; \
	FILES=`find $$cwd \( $(SRCS) \) -print`; \
	/bin/tar hcvozf $$cwd.tar.gz $$FILES; \
	mv $$cwd.tar.gz $$cwd

new: c default
c: clean
clean:
	/bin/rm -fr *.o *~ $(LIB) $(APP)
	cd Draggers; $(MAKE) clean
	cd Objects; $(MAKE) clean
	cd Examples; $(MAKE) clean
	/bin/rm -fr `file * | grep ELF | awk -F: ' {print $$1}'`
 
tag:
	TAG="STABLE_"`date +%m_%d_%y`;\
	cvs tag $$TAG .

