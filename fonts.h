#ifndef _FONTS_H_
#define _FONTS_H_

typedef struct t_rect {
	int left;
	int top;
	int right;
	int bot;
	int width;
	int height;
	int center;
	int centerx;
	int centery;
	//0-use texture maps  1-use gl_points in a loop
	int glpoints;
	int pt_size;
} Rect;
extern void initialize_fonts(void);
extern void cleanup_fonts(void);
extern void ggprint(Rect *r, int ptsz,
								int advance, int cref, const char *fmt, ...);
#endif //_FONTS_H_

