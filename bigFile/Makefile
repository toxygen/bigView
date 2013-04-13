
include Global.defs

LIB   = libBF.a
LIB_H = bigFile.h
LIB_O = bigFile.o

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
# Standard rules: clean, co, install
##############################################################################

new: c default
c: clean
clean:
	/bin/rm -fr *.o *~ *.bak $(LIB) $(APP)

SRCS = -name '*.[cChf]' -o -name Makefile  
tar:
	FILES=`find . \( $(SRCS) \) -print`; \
	/bin/tar cvof bigFile.tar $$FILES; 

##############################################################################
# include RCS rules
##############################################################################
 
localDepend: Makefile
	makedepend -Y. $(SRC) $(LOCAL_FLAGS) -flocalDepend 2> /dev/null

include localDepend
