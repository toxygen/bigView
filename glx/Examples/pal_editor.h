#ifndef _PAL_EDITOR_H_
#define _PAL_EDITOR_H_

#include <vector>

namespace Pal {
  typedef void(* UserFunc )(double (*)[4],void*);
  typedef std::pair<UserFunc,void*> UserPair;
  
  void paltool(XtAppContext app, Display* display);
  void addUserFunc(Pal::UserFunc, void* user=0);

  void callUserFuncs(void);

  extern std::vector< Pal::UserPair > UserFuncs;
  
};

extern unsigned int thepal[256];  /* tha actual palette being edited */

extern int edit[4];	/* flags */
extern int view[4];	/* flags */
extern int pins[2];		/* limits */
extern double  pal[256][4];  	/* the LUT being editted/displayed */
extern unsigned int *ipal;	/* aabbggrr */
extern int d_histo[256];

extern    Widget	pe_Ered;
extern    Widget	pe_Egreen;
extern    Widget	pe_Eblue;
extern    Widget	pe_Ealpha;
extern    Widget	pe_Vred;
extern    Widget	pe_Vgreen;
extern    Widget	pe_Vblue;
extern    Widget	pe_Valpha;
extern    Widget	palette_drawn;

void net_send(void);
void net_get(void);

// pal_wid.c
void net_putcmap(void);
void compute_histogram(void);
int send_command(char *cmd);

void dtoipal(int i);
void itodpal(int i);
void convertpalto(int i);
int do_pal_tool(Widget w);
void do_update(void);
void draw_pal_vector(int k);
void draw_partial_pal_vector(int k,int st,int ed);
void draw_pins(Widget w);
void draw_text(Widget w);
void changelut(off_t i);
void freehand_tool(int ox,int mx,int oy,int my,Widget w,Pixel bp);
void rotate_tool(int ox,int mx,int oy,int my,Widget w,Pixel bp);
void fiddle_tool(int ox,int mx,int oy,int my,Widget w,Pixel bp);
void gamma_tool(int ox,int mx,int oy,int my,Widget w,Pixel bp);
void setit(int i, double f);
void interpit(int nl, int nr);
void do_palette(Widget w,XtPointer client,XtPointer call);

void paled_option(Widget w,XtPointer client,XtPointer call);
void paled_file(Widget w,XtPointer client,XtPointer call);
void toggle_view(Widget w,XtPointer  client,XtPointer call);
void toggle_edit(Widget w,XtPointer client,XtPointer call);
void sel_new_lut(Widget w,XtPointer client,XtPointer call);
void do_fixed_pal(Widget w,XtPointer client, XtPointer call);

void setlut(off_t i,unsigned int *buf);

void RGB_to_HSL(double r,double g,double b,double *h,double *s,double *l);
void HSL_to_RGB(double h,double sl,double l,double *r,double *g,double *b);
void RGB_to_HSV(double r,double g,double b,double *h,double *s,double *v);
void HSV_to_RGB(double h1,double s1,double v1,double *r,double *g,double *b);

int getglxctx(Display *dpy,int *depth, Visual **visual);

#endif
