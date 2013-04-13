
##############################################################################
# GLOBAL COMPILER FLAGS
##############################################################################

include Global.defs

##############################################################################
# LOCAL COMPILER FLAGS
##############################################################################

LOCAL_FLAGS = -D_FILE_OFFSET_BITS=64 -Iglx -IbigFile -Ippm

LINK  = -Lglx -lGlx -Lppm -lPPM -LbigFile -lBF \
	-lGLw -lGL -lXm -lXt -lXext -lX11 \
	-lpthread -lm $(GLBL_LINK)

OBJS = glerr.o PageManager.o PageReader.o threadedQueue.o Net.o
APPS = showPaged genPaged browse browser

default: sublibs $(OBJS) 
	$(MAKE) $(APPS)

sublibs:
	cd bigFile; $(MAKE) 
	cd ppm; $(MAKE) 
	cd glx; $(MAKE)

showPaged: showPaged.o $(OBJS)
	$(CPLUSPLUS) -o $@ $(GLBL_FLAGS) showPaged.o $(OBJS) $(LINK)

genPaged: genPaged.o $(OBJS)
	$(CPLUSPLUS) -o $@ $(GLBL_FLAGS) genPaged.o $(OBJS) $(LINK)

browse: browse.o $(OBJS)
	$(CPLUSPLUS) -o $@ $(GLBL_FLAGS) browse.o $(OBJS) $(LINK)

browser: browser.o $(OBJS)
	$(CPLUSPLUS) -o $@ $(GLBL_FLAGS) browser.o $(OBJS) $(LINK)

SRCS = -name '*.[cChf]' \
	-o -name '*.cxx' \
	-o -name '*.pl' \
	-o -name README \
	-o -name NOSA \
	-o -name Makefile \
	-o -name Global.defs 

tar:
	cd ../; \
	FILES=`find bigView -follow \( $(SRCS) \) -print`; \
	/bin/tar chvofz bigView.tar.gz $$FILES; \
	/bin/mv bigView.tar.gz bigView


##############################################################################
# Standard rules: clean, co, install
##############################################################################
new: c default
c: clean
clean: 
	cd glx; $(MAKE) clean
	cd bigFile; $(MAKE) clean
	cd ppm; $(MAKE) clean	
	/bin/rm -fr junk* *.o *~ *.rpt *.gz *.out core* $(APPS) $(OBJS)
	/bin/rm -fr `file * | grep ELF | awk -F: ' {print $$1}'`

tag:
	TAG="BIGVIEWSTABLE_"`date +%m_%d_%y`;\
	cvs tag $$TAG .

