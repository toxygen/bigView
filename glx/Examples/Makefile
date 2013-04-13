include ../Global.defs

VT  = /home/sandstro/visTools
VT_FLAGS = -I$(VT)/GelLib -I$(VT)/support/ref -I$(VT)/FEL -I$(VT)/MultiTask
VT_LINK = -L$(VT)/lib -lGEL -lFEL -lTask
GLX = /home/sandstro/shop/glx
LIB = ../libGlx.a
GLX_FLAGS = -I$(GLX) -DGL_GLEXT_PROTOTYPES
GLX_LINK  = -L$(GLX) -lGlx
#FTGLINC   = -I/usr/local/include/FTGL -I/usr/include/freetype2
#FTGLLNK   = -L/usr/local/lib -lXi -lfreetype -lftgl
FTGLINC   =
FTGLLNK   =

#PROF = -pg
PROF =
LOCAL_FLAGS = $(PROF) -LANG:std -DOPEN_GL $(GLX_FLAGS) $(VT_FLAGS) $(FTGLINC)
LINK = $(PROF) $(GLBL_FLAGS) $(GLX_LINK) $(VT_LINK) $(FTGLLNK) \
	-lGLw -lGL -lGLU \
	-lXm -lXt -lXext -lX11 -lm $(GLBL_LINK) -lpthread

SRC = boxes.C cmap.C drag.C glxAtomic.C glxContext.C glxContextThread.C \
	glxFrustum.C glxThread.C offscreen.C packing.C pix2d.C \
	plotPoints.C q.C qTest.C rotateCamera.C \
	trackball.C twod.C vert.C work.C \
	vecTest.C slider.C hist.C colorbar.C plot.C cross2d.C tfedit.C \
	drag3d.C

OBJ = $(SRC:.C=.o) 
APP = akima boxes clip cmap colorbar cross2d cross3d check cube	\
drag3d drag dump ellipse ext eye glxAtomic glxContext glxContextThread	\
glxFrustum glxThread hat hex hist latlon load loadAlpha minimum mt	\
noborder oct oct2 offscreen p3d packing pix2d plot plotPoints pcoords	\
pspace q rect qTest rotateCamera select2d selectrect slider sphere	\
tfedit trackball twod tuple vecTest vert vec3d wakeup wall	\
work tri wake

all: $(APP)

a3: $(LIB) a3.o
	g++ -o $@ $@.o $(LINK)

akima: $(LIB) akima.o
	g++ -o $@ $@.o $(LINK)
bc: $(LIB) bc.o
	g++ -o $@ $@.o $(LINK)
boxes: $(LIB) boxes.o
	g++ -o $@ $@.o $(LINK)
box3d: $(LIB) box3d.o
	g++ -o $@ $@.o $(LINK)
clip: $(LIB) clip.o
	g++ -o $@ $@.o $(LINK)
cmap: $(LIB) cmap.o
	g++ -o $@ $@.o $(LINK)
colorbar: $(LIB) colorbar.o
	g++ -o $@ $@.o $(LINK)
cross2d: $(LIB) cross2d.o
	g++ -o $@ $@.o $(LINK)
cross3d: $(LIB) cross3d.o
	g++ -o $@ $@.o $(LINK)
cedit: $(LIB) cedit.o
	g++ -o $@ $@.o $(LINK)
check: $(LIB) check.o
	g++ -o $@ $@.o $(LINK)
cube: $(LIB) cube.o
	g++ -o $@ $@.o $(LINK)
drag3d: $(LIB) drag3d.o
	g++ -o $@ $@.o $(LINK)
drag: $(LIB) drag.o
	g++ -o $@ $@.o $(LINK)
dump: $(LIB) dump.o
	g++ -o $@ $@.o $(LINK)
ellipse: $(LIB) ellipse.o
	g++ -o $@ $@.o $(LINK)
ext: $(LIB) ext.o
	g++ -o $@ $@.o $(LINK)
eye: $(LIB) eye.o
	g++ -o $@ $@.o $(LINK)
fbo: $(LIB) fbo.o
	g++ -o $@ $@.o $(LINK)
fbo2: $(LIB) fbo2.o
	g++ -o $@ $@.o $(LINK)
glxAtomic: $(LIB) glxAtomic.o
	g++ -o $@ $@.o $(LINK)
glxContext: $(LIB) glxContext.o
	g++ -o $@ $@.o $(LINK)
glxContextThread: $(LIB) glxContextThread.o
	g++ -o $@ $@.o $(LINK)
glxFrustum: $(LIB) glxFrustum.o
	g++ -o $@ $@.o $(LINK)
glxThread: $(LIB) glxThread.o
	g++ -o $@ $@.o $(LINK)
hat: $(LIB) hat.o
	g++ -o $@ $@.o $(LINK)
hex: $(LIB) hex.o
	g++ -o $@ $@.o $(LINK)
hist: $(LIB) hist.o
	g++ -o $@ $@.o $(LINK)
inset: $(LIB) inset.o
	g++ -o $@ $@.o $(LINK)
latlon: $(LIB) latlon.o
	g++ -o $@ $@.o $(LINK)
load: $(LIB) load.o
	g++ -o $@ $@.o $(LINK)
loadAlpha: $(LIB) loadAlpha.o
	g++ -o $@ $@.o $(LINK)
messages: $(LIB) messages.o
	g++ -o $@ $@.o $(LINK)
minimum: $(LIB) minimum.o
	g++ -o $@ $@.o $(LINK)
mplayer_target: $(LIB) mplayer_target.o
	g++ -o $@ $@.o $(LINK)
mt: $(LIB) mt.o
	g++ -o $@ $@.o $(LINK)
noborder: $(LIB) noborder.o
	g++ -o $@ $@.o $(LINK)
oct: $(LIB) oct.o
	g++ -o $@ $@.o $(LINK)
oct2: $(LIB) oct2.o
	g++ -o $@ $@.o $(LINK)
offscreen: $(LIB) offscreen.o
	g++ -o $@ $@.o $(LINK)
p3d: $(LIB) p3d.o
	g++ -o $@ $@.o $(LINK)
packing: $(LIB) packing.o
	g++ -o $@ $@.o $(LINK)
pix2d: $(LIB) pix2d.o
	g++ -o $@ $@.o $(LINK)
plot: $(LIB) plot.o
	g++ -o $@ $@.o $(LINK)
plotPoints: $(LIB) plotPoints.o
	g++ -o $@ $@.o $(LINK)
pcoords: $(LIB) pcoords.o
	g++ -o $@ $@.o $(LINK)
pspace: $(LIB) pspace.o NodeBlock.o iTuple.o
	g++ -o $@ $@.o NodeBlock.o iTuple.o $(LINK)
q: $(LIB) q.o
	g++ -o $@ $@.o $(LINK)
radio: $(LIB) radio.o
	g++ -o $@ $@.o $(LINK)
rect: $(LIB) rect.o
	g++ -o $@ $@.o $(LINK)
qTest: $(LIB) qTest.o
	g++ -o $@ $@.o $(LINK)
rotateCamera: $(LIB) rotateCamera.o
	g++ -o $@ $@.o $(LINK)
select2d: $(LIB) select2d.o 
	g++ -o $@ $@.o $(LINK)
selectrect: $(LIB) selectrect.o 
	g++ -o $@ $@.o $(LINK)
slider: $(LIB) slider.o
	g++ -o $@ $@.o $(LINK)
shader: $(LIB) shader.o
	g++ -o $@ $@.o $(LINK)
sphere: $(LIB) sphere.o
	g++ -o $@ $@.o $(LINK)
sprite: $(LIB) sprite.o adj.o
	g++ -o $@ $@.o adj.o $(LINK)
srt: $(LIB) srt.o
	g++ -o $@ $@.o $(LINK)
subvol: $(LIB) subvol.o slicer.o 
	g++ -o $@ $@.o slicer.o $(LINK)
spr: $(LIB) spr.o adj.o
	g++ -o $@ $@.o adj.o $(LINK)
#testTextureCmap: $(LIB) testTextureCmap.o
#	g++ -o $@ $@.o $(LINK)
tfedit: $(LIB) tfedit.o
	g++ -o $@ $@.o $(LINK)
trackball: $(LIB) trackball.o
	g++ -o $@ $@.o $(LINK)
twod: $(LIB) twod.o
	g++ -o $@ $@.o $(LINK)
tuple: $(LIB) tuple.o iTuple.o
	g++ -o $@ $@.o iTuple.o $(LINK)
vecTest: $(LIB) vecTest.o
	g++ -o $@ $@.o $(LINK)
vert: $(LIB) vert.o
	g++ -o $@ $@.o $(LINK)
vec3d: $(LIB) vec3d.o
	g++ -o $@ $@.o $(LINK)
viewTek: $(LIB) viewTek.o str.o
	g++ -o $@ $@.o str.o $(LINK)
vol: $(LIB) vol.o slicer.o 
	g++ -o $@ $@.o slicer.o $(LINK)

# wakeup sometimes needs: g++ -o $@ $@.o $(LINK) -ldl -lreadline -lhistory 
wakeup: $(LIB) wakeup.o
	g++ -o $@ $@.o $(LINK) -ldl -lreadline -lhistory 
wall: $(LIB) wall.o 
	g++ -o $@ $@.o $(LINK)
wake: $(LIB) wake.o
	g++ -o $@ $@.o $(LINK)
work: $(LIB) work.o
	g++ -o $@ $@.o $(LINK)
xyscale: $(LIB) xyscale.o
	g++ -o $@ $@.o $(LINK)

tri: $(LIB) tri.o
	g++ -o $@ $@.o $(LINK)
v: $(LIB) v.o
	g++ -o $@ $@.o $(LINK)

##############################################################################
# Standard rules: clean, co, install
##############################################################################

new: c default
c: clean
clean:
	/bin/rm -fr *.o *~  *.bak
	/bin/rm -fr `file * | grep ELF | awk -F: ' {print $$1}'`

SRCS = -name '*.[cChf]' -o -name Makefile  
tar:
	FILES=`find . \( $(SRCS) \) -print`; \
	/bin/tar cvof glx.tar $$FILES; 


