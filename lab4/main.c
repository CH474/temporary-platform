#include <stdio.h>
#include "../common/common.h"

#define COLOR_BACKGROUND	FB_COLOR(0xff,0xff,0xff)
static int touch_fd;
//int pressx=0,pressy=0;
static void touch_event_cb(int fd)
{
	int type,x,y,finger;
	int lastX[5]={0,0,0,0,0},lastY[5]={0,0,0,0,0};
	int pressx,pressy,pressfinger;
	type = touch_read(fd, &x,&y,&finger);
	//if(x>0&&x<1000)pressx=x;
	//if(y>0&&y<1000)pressy=y;
	//printf("TOUCH type=%d x=%d y=%d finger=%d\n",type,x,y,finger);
	switch(type){
	case TOUCH_PRESS:
		printf("TOUCH_PRESS：x=%d,y=%d,finger=%d\n",x,y,finger);
	
		pressx=x;
		pressy=y;
		pressfinger=finger;
		//fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
		//int i=0;
		//for(i=0;i<5;i++){
		//	if(x[i]>=10){
		//	fb_draw_circle(x[i],y[i],10,0x66ccff);
		//	}
		//}
		//fb_update();
		break;
	case TOUCH_MOVE:
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
		int color=0,radius=0;
		switch(finger){
			case 0:
				radius=4;
				color=0x66ccff;
				break;
			case 1:
				radius=4;
				color=0xdd0000;
				break;
			case 2:
				radius=4;
				color=0x00dd00;
				break;
			case 3:
				radius=4;
				color=0x0000dd;
				break;
			case 4:
				radius=4;
				color=0xdddddd;
				break;
			}
		fb_draw_rect(x,y,radius,radius,color);
		//fb_update();
		break;
	case TOUCH_RELEASE:
		printf("TOUCH_RELEASE：x=%d,y=%d,finger=%d\n",x,y,finger);
		if(x<100&&y<100){
			fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
			//fb_update();
		}
		break;
	case TOUCH_ERROR:
		printf("close touch fd\n");
		close(fd);
		task_delete_file(fd);
		break;
	default:
		return;
	}
	lastX[0]=pressx;
	lastY[0]=pressy;
	fb_update();
	return;
}

int main(int argc, char *argv[])
{
	fb_init("/dev/fb0");
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	fb_update();
	
	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event3");
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	task_add_file(touch_fd, touch_event_cb);
	
	task_loop(); //进入任务循环
	return 0;
}
