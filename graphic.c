#include "common.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>


static int LCD_FB_FD;
static int *LCD_FB_BUF = NULL;
static int *LCD_FB_FRONT, *LCD_FB_BACK;
struct fb_var_screeninfo LCD_FB_VAR;
static int DRAW_BUF[SCREEN_WIDTH*SCREEN_HEIGHT];

static struct area {
	int x1, x2, y1, y2;
} update_area = {0,0,0,0};

#define AREA_SET_EMPTY(pa) do {\
	(pa)->x1 = SCREEN_WIDTH;\
	(pa)->x2 = 0;\
	(pa)->y1 = SCREEN_HEIGHT;\
	(pa)->y2 = 0;\
} while(0)

void fb_init(char *dev)
{
	int fd;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;

	if(LCD_FB_BUF != NULL) return; /*already done*/

	//First: Open the device
	if((fd = open(dev, O_RDWR)) < 0){
		printf("Unable to open framebuffer %s, errno = %d\n", dev, errno);
		return;
	}
	if(ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix) < 0){
		printf("Unable to FBIOGET_FSCREENINFO %s\n", dev);
		return;
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) < 0){
		printf("Unable to FBIOGET_VSCREENINFO %s\n", dev);
		return;
	}

	printf("framebuffer info: bits_per_pixel=%u,size=(%d,%d),virtual_pos_size=(%d,%d)(%d,%d),line_length=%u,smem_len=%u\n",
		fb_var.bits_per_pixel, fb_var.xres, fb_var.yres, fb_var.xoffset, fb_var.yoffset,
		fb_var.xres_virtual, fb_var.yres_virtual, fb_fix.line_length, fb_fix.smem_len);

	//Second: mmap
	int *addr;
	addr = mmap(NULL, fb_fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if((int)addr == -1){
		printf("failed to mmap memory for framebuffer.\n");
		return;
	}

	if((fb_var.xoffset != 0) ||(fb_var.yoffset != 0))
	{
		fb_var.xoffset = 0;
		fb_var.yoffset = 0;
		if(ioctl(fd, FBIOPAN_DISPLAY, &fb_var) < 0) {
			printf("FBIOPAN_DISPLAY framebuffer failed\n");
		}
	}


	LCD_FB_FD = fd;
	LCD_FB_BUF = addr;
	LCD_FB_FRONT = addr;
	LCD_FB_BACK = addr + fb_var.xres*fb_var.yres;
	LCD_FB_VAR = fb_var;

	//set empty
	AREA_SET_EMPTY(&update_area);
	return;
}

static void _copy_area(int *dst, int *src, struct area *pa)
{
	int x, y, w, h;
	x = pa->x1; w = pa->x2-x;
	y = pa->y1; h = pa->y2-y;
	src += y*SCREEN_WIDTH + x;
	dst += y*SCREEN_WIDTH + x;
	while(h-- > 0){
		memcpy(dst, src, w*4);
		src += SCREEN_WIDTH;
		dst += SCREEN_WIDTH;
	}
}

static int _check_area(struct area *pa)
{
	if(pa->x2 == 0) return 0; //is empty

	if(pa->x1 < 0) pa->x1 = 0;
	if(pa->x2 > SCREEN_WIDTH) pa->x2 = SCREEN_WIDTH;
	if(pa->y1 < 0) pa->y1 = 0;
	if(pa->y2 > SCREEN_HEIGHT) pa->y2 = SCREEN_HEIGHT;

	if((pa->x2 > pa->x1) && (pa->y2 > pa->y1))
		return 1; //no empty

	//set empty
	AREA_SET_EMPTY(pa);
	return 0;
}

void fb_update(void)
{
	if(_check_area(&update_area) == 0) return; //is empty
	_copy_area(LCD_FB_FRONT, DRAW_BUF, &update_area);
	AREA_SET_EMPTY(&update_area); //set empty
	return;
}

/*======================================================================*/

static void * _begin_draw(int x, int y, int w, int h)
{
	int x2 = x+w;
	int y2 = y+h;
	if(update_area.x1 > x) update_area.x1 = x;
	if(update_area.y1 > y) update_area.y1 = y;
	if(update_area.x2 < x2) update_area.x2 = x2;
	if(update_area.y2 < y2) update_area.y2 = y2;
	return DRAW_BUF;
}

void fb_draw_pixel(int x, int y, int color)
{
	if(x<0 || y<0 || x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT) return;
	int *buf = _begin_draw(x,y,1,1);
/*---------------------------------------------------*/
	*(buf + y*SCREEN_WIDTH + x) = color;
/*---------------------------------------------------*/
	return;
}

void fb_draw_rect(int x, int y, int w, int h, int color)
{
	if(x < 0) { w += x; x = 0;}
	if(x+w > SCREEN_WIDTH) { w = SCREEN_WIDTH-x;}
	if(y < 0) { h += y; y = 0;}
	if(y+h >SCREEN_HEIGHT) { h = SCREEN_HEIGHT-y;}
	if(w<=0 || h<=0) return;
	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------*/
	//printf("you need implement fb_draw_rect()\n"); exit(0);
	// Add your code here
	//(buf+y*SCREEN_WIDTH+x)=color;
	int currentx,currenty;
	for(currentx=x;currentx<=x+w;currentx++)
		for(currenty=y;currenty<=y+h;currenty++)
			fb_draw_pixel(currentx,currenty,color);
/*---------------------------------------------------*/
	return;
}

void fb_draw_line(int x1, int y1, int x2, int y2, int color)
{
/*---------------------------------------------------*/
	//printf("you need implement fb_draw_line()\n"); exit(0);
	// Add your code here
	int currentx,currenty;
	//if(x1<0){x2-=x1;x1=0;}
	//if(y1<0){y2-=y1;y1=0;}
	//if(x2>SCREEN_WIDTH){x2=SCREEN_WIDTH;}
	//if(y2>SCREEN_HEIGHT){y2=SCREEN_HEIGHT;}
	int mid;
	if(x1>x2){
		mid=x1;
		x1=x2;
		x2=mid;
		mid=y1;
		y1=y2;
		y2=mid;
	}
	int sy,ey,h;
	if(y1>y2){
		sy=y2;
		ey=y1;
		h=y1-y2;
	}
	else{
		sy=y1;
		ey=y2;
		h=y2-y1;
	}
	int * buf=_begin_draw(x1,sy,x2-x1,h);
	if(x1!=x2){
		float k=(float)(y2-y1)/(float)(x2-x1);
		int cx,cy;
		for(cx=x1;cx<=x2;cx++){
			cy=y1+(int)((cx-x1)*k);
			fb_draw_pixel(cx,cy,color);
		}
	}
	else{
		int cy;
		for(cy=sy;cy<=ey;cy++){
			fb_draw_pixel(x1,cy,color);
		}
	}
/*---------------------------------------------------*/
	return;
}

void fb_draw_image(int x, int y, fb_image *image, int color)
{
	if(image == NULL) return;

	int ix = 0; //image x
	int iy = 0; //image y
	int w = image->pixel_w; //draw width
	int h = image->pixel_h; //draw height

	if(x<0) {w+=x; ix-=x; x=0;}
	if(y<0) {h+=y; iy-=y; y=0;}
	
	if(x+w > SCREEN_WIDTH) {
		w = SCREEN_WIDTH - x;
	}
	if(y+h > SCREEN_HEIGHT) {
		h = SCREEN_HEIGHT - y;
	}
	if((w <= 0)||(h <= 0)) return;

	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------------------*/
	int *dst = (int *)(buf + y*SCREEN_WIDTH + x);
	char *src = image->content + iy*image->line_byte + ix*4;
/*---------------------------------------------------------------*/

	int alpha;
	int ww;
	if(image->color_type == FB_COLOR_RGB_8880) /*lab3: jpg*/
	{
		//printf("you need implement fb_draw_image() FB_COLOR_RGB_8880\n"); exit(0);
		// Add your code here
		int size=(image->line_byte>=4*w)?4*w:image->line_byte;
		for(ww=0;ww<h;++ww)
		{
			memcpy(dst,src,size);
			dst+=SCREEN_WIDTH;
			src+=image->line_byte;
		}
		return;
	}

	if(image->color_type == FB_COLOR_RGBA_8888) /*lab3: png*/
	{
		//printf("you need implement fb_draw_image() FB_COLOR_RGBA_8888\n"); exit(0);
		// Add your code here
		int i,j;
		char r,g,b,alpha;
		for(i=y;i<y+h;i++){
			for(j=x;j<x+w;j++){
				int* temp=(int*)(buf+i*SCREEN_WIDTH+j);
				char* p=temp;
				char* pcolor=image->content+(i-y)*image->line_byte+(j-x)*4;
				alpha=*(char*)(pcolor+3);
				r=*(char*)(pcolor+2);
				g=*(char*)(pcolor+1);
				b=*(char*)pcolor;
				p[0]+=(((b-p[0])*alpha)>>8);
				p[1]+=(((g-p[1])*alpha)>>8);
				p[2]+=(((r-p[2])*alpha)>>8);
			}
		}
		return;
	}

	if(image->color_type == FB_COLOR_ALPHA_8) /*lab3: font*/
	{
		// printf("you need implement fb_draw_image() FB_COLOR_ALPHA_8\n"); exit(0);
		// Add your code here
		char *i;
		int *j;
		int c,r,g,b,alpha;
		for(ww=0;ww<h;ww++){
			i=src;
			j=dst;
			for(c=0;c<w;c++){
				alpha=*(char*)i;
				r=(color>>16)&0xff;
				g=(color>>8)&0xff;
				b=color&0xff;
				*(char*)j+=((b-*(char *)j)*alpha)>>8;
				*((char*)(j)+1)+=((g-*((char*)(j)+1))*alpha)>>8;
				*((char*)(j)+2)+=((r-*((char*)(j)+2))*alpha)>>8;
				i++;
				j++;
			}
			dst+=SCREEN_WIDTH;
			src+=image->line_byte;
		}
		return;
	}
/*---------------------------------------------------------------*/
	return;
}

void fb_draw_border(int x, int y, int w, int h, int color)
{
	if(w<=0 || h<=0) return;
	fb_draw_rect(x, y, w, 1, color);
	if(h > 1) {
		fb_draw_rect(x, y+h-1, w, 1, color);
		fb_draw_rect(x, y+1, 1, h-2, color);
		if(w > 1) fb_draw_rect(x+w-1, y+1, 1, h-2, color);
	}
}

/** draw a text string **/
void fb_draw_text(int x, int y, char *text, int font_size, int color)
{
	fb_image *img;
	fb_font_info info;
	int i=0;
	int len = strlen(text);
	while(i < len)
	{
		img = fb_read_font_image(text+i, font_size, &info);
		if(img == NULL) break;
		fb_draw_image(x+info.left, y-info.top, img, color);
		fb_free_image(img);

		x += info.advance_x;
		i += info.bytes;
	}
	return;
}

void fb_draw_circle(int x,int y,int r,int color){
	if(x-r<0||x+r>SCREEN_WIDTH||y-r<0||y+r>SCREEN_HEIGHT)return;
	int * buf=_begin_draw(x-r,y-r,x+r,y+r);
	int i,j;
	for(i=y-r;i<=y+r;i++){
		for(j=x-r;j<x+r;j++){
			if((i-y)*(i-y)+(j-x)*(j-x)<=r*r)
				fb_draw_pixel(j,i,color);
		}
	}
}

void fb_draw_thick_line(int x1,int y1,int x2,int y2, int r,int color){
	int i;
	int dy,dx;
	dy=y2>y1?y2-y1:y1-y2;
	dx=x2>x1?x2-x1:x1-x2;
	int steep=dy>dx;
	if(steep){
		double slope=(double)(x2-x1)/(y2-y1);
		int mid;
		if(y2<y1){
			mid=y2;
			y2=y1;
			y1=mid;
		}
		for(i=y1;i<=y2;i++){
			fb_draw_circle((int)((double)x1+slope*(i-y1)),i,r,color);
		}
	}
	else{
		double slope=(double)(y2-y1)/(x2-x1);
		int mid;
		if(x2<x1){
			mid=x2;
			x2=x1;
			x1=mid;
		}
		for(i=x1;i<=x2;i++){
			fb_draw_circle(i,(int)((double)y1+slope*(i-x1)),r,color);
		}
	}
}









