##############################################################################
# DEFINES
##############################################################################

include Global.defs

##############################################################################
# LOCAL COMPILER FLAGS
##############################################################################

LOCAL_FLAGS= -I../ \
	-I../../Env3D \
	-I../../VisTech \
	-I../../MultiTask \
	-I../../GelLib \
	-I../../FEL \
	-I../socket \
	-I../bigFile \
	-I../gen \
	-I../ref 

##############################################################################
# LIBRARY DEFINES
##############################################################################

LIB       = libPPM.a
LIB_O     = ppm.o
LIB_H     = ppm.h
LIB_C     = $(LIB_O:.o=.C)

TST       = viewPPM ppmcat ppmhdr writeTest tex

default: $(LIB) 

$(LIB): $(LIB_O) $(LIB_H)
	-@ $(CPLUSPLUS) $(GLBL_FLAGS) $(LOCAL_FLAGS) $(LIB_O) 2> /dev/null
	$(AR) $(ARFLAGS) $(LIB) $(LIB_O) 
	$(RANLIB) $(LIB)
	$(MAKE) makeLink

makeLink:
	TARGET=`pwd`"/$(LIB)"; \
	export TARGET; \
	echo "TARGET="$$TARGET; \
	cd $(LIBDIR); /bin/rm -fr $(LIB); ln -s $$TARGET $(LIB) 

##############################################################################
# Test app rules
##############################################################################

LNK     = -lPPM -lBF \
	-lEnv3D -lGEL -lGLw -lGL -lGLU \
	-lXm -lXt -lXext -lX11 -lm $(GLBL_LINK)

viewPPM: viewPPM.o $(LIB)
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o $(LNK)

viewStereo: viewStereo.o $(LIB)
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o $(LNK)

ppmhdr: ppmhdr.o $(LIB)
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o $(LNK)

ppmcat: ppmcat.o $(LIB) 
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o $(LNK)

ppminterp: ppminterp.o $(LIB) 
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o $(LNK)

writeTest: writeTest.o $(LIB)
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o $(LNK)

tex: tex.o $(LIB) glerr.o
	$(CPLUSPLUS) $(GLBL_FLAGS) -o $@ $@.o glerr.o $(LNK)

##############################################################################
# Standard rules: clean, co, install
##############################################################################
new: c default
c: clean
clean: 
	touch ii_files junk.o junk~ junk.rpt junk.out core $(TST)
	/bin/rm -fr *.a *.so junk* ii_files *.o *~ *.rpt *.out core* $(TST)
	/bin/rm -fr `file * | grep ELF | awk -F: ' {print $$1}'`

SRCS = -name '*.[cChf]' -o -name Makefile  
tar:
	FILES=`find . \( $(SRCS) \) -print`; \
	/bin/tar cvof ppm.tar $$FILES; 
