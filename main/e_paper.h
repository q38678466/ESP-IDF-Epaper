#ifndef __E_PAPER_H__
#define __E_PAPER_H__

#include "stdint.h"

#define EPD_W	152 
#define EPD_H	152

#define WHITE 0xFF
#define BLACK 0x00



#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

typedef struct {
	u8 *Image;
	u16 width;
	u16 height;
	u16 widthMemory;
	u16 heightMemory;
	u16 color;
	u16 rotate;
	u16 widthByte;
	u16 heightByte;
	
}PAINT;
extern PAINT Paint;
extern u8 ImageBW[2888];

//定义E-Paper显示方向 
/*******************
Rotaion:0-0度方向
Rotaion:90-90度方向
Rotaion:180-180度方向
Rotaion:270-270度方向
*******************/
#define Rotation 180  

void epaper_spi_init();
void EPD_Init(void);
void EPD_FastMode1Init();
void EPD_Display_Clear();
void EPD_FastUpdate(void);
void EPD_Clear_R26H(void);
void EPD_Display(const u8 *image);
void EPD_Update(void);
void EPD_PartUpdate(void);
void EPD_DeepSleep(void);
void Paint_NewImage(u8 *image,u16 Width,u16 Height,u16 Rotate,u16 Color); 					 //创建画布控制显示方向
void Paint_SetPixel(u16 Xpoint,u16 Ypoint,u16 Color);
void Paint_Clear(u8 Color);
void EPD_DrawLine(u16 Xstart,u16 Ystart,u16 Xend,u16 Yend,u16 Color);
void EPD_DrawRectangle(u16 Xstart,u16 Ystart,u16 Xend,u16 Yend,u16 Color,u8 mode);  //画矩形
void EPD_DrawCircle(u16 X_Center,u16 Y_Center,u16 Radius,u16 Color,u8 mode);        //画圆
void EPD_ShowChar(u16 x,u16 y,u16 chr,u16 size1,u16 color);                         //显示字符
void EPD_ShowString(u16 x,u16 y,u8 *chr,u16 size1,u16 color);                       //显示字符串
void EPD_ShowNum(u16 x,u16 y,u32 num,u16 len,u16 size1,u16 color);                  //显示数字
void EPD_ShowPicture(u16 x,u16 y,u16 sizex,u16 sizey,const u8 BMP[],u16 Color);			//显示图片      
void EPD_ClearWindows(u16 xs,u16 ys,u16 xe,u16 ye,u16 color);
void EPD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u8 pre,u8 sizey,u8 color);
void EPD_ShowWatch(u16 x,u16 y,float num,u8 len,u8 pre,u8 sizey,u8 color);
void EPD_ShowNum_Two(u16 x, u16 y, u16 num1, u8 sizey, u8 color);
void EPD_ShowSensor_Data(u16 x,u16 y,float num,u8 len,u8 pre,u8 sizey,u8 color);
#endif