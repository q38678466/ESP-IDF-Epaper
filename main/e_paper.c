#include "e_paper.h"
#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "EPD_Font.h"
#include "Pic.h"
PAINT Paint;
// SPI Bus
#define EPD_PANEL_SPI_CLK           1000000
#define EPD_PANEL_SPI_CMD_BITS      8
#define EPD_PANEL_SPI_PARAM_BITS    8
#define EPD_PANEL_SPI_MODE          0
// e-Paper GPIO
#define EXAMPLE_PIN_NUM_EPD_DC      14
#define EXAMPLE_PIN_NUM_EPD_RST     12
#define EXAMPLE_PIN_NUM_EPD_CS      27
#define EXAMPLE_PIN_NUM_EPD_BUSY    13
// e-Paper SPI
#define EXAMPLE_PIN_NUM_MOSI        26
#define EXAMPLE_PIN_NUM_SCLK        25

static const char *TAG = "epaper_demo_plain";
#define EPAPER_SPI_HOST    SPI2_HOST
spi_device_handle_t spi;



void EPD_DC_Clr()
{
    gpio_set_level(EXAMPLE_PIN_NUM_EPD_DC, 0);
}

void EPD_DC_Set()
{
    gpio_set_level(EXAMPLE_PIN_NUM_EPD_DC, 1);
}

void EPD_WR_REG(u8 cmd)
{
    EPD_DC_Clr();
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8;                   //Command is 8 bits
    t.tx_buffer = &cmd;             //The data is the cmd itself
    t.user = (void*)0;              //D/C needs to be set to 0
    // if (keep_cs_active) {
    //     t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
    // }
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    EPD_DC_Set();
    assert(ret == ESP_OK);          //Should have had no issues.

}

void EPD_WR_DATA8(const uint8_t data)
{
    EPD_DC_Set();
    esp_err_t ret;
    spi_transaction_t t;
    // if (1 == 0) {
    //     return;    //no need to send anything
    // }
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 1 * 8;             //Len is in bytes, transaction length is in bits.
    t.tx_buffer = &data;             //Data
    t.user = (void*)1;              //D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    EPD_DC_Set();
    assert(ret == ESP_OK);          //Should have had no issues.
}

void EPD_READBUSY(void)
{
  while(1)
  {
    if(gpio_get_level(EXAMPLE_PIN_NUM_EPD_BUSY)==0)
    {
      break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void EPD_RES_Clr(void)
{
  gpio_set_level(EXAMPLE_PIN_NUM_EPD_RST, 0);
}

void EPD_RES_Set(void)
{
  gpio_set_level(EXAMPLE_PIN_NUM_EPD_RST, 1);
}

/*******************************************************************
    函数说明:硬件复位函数
    入口参数:无
    说明:在E-Paper进入Deepsleep状态后需要硬件复位  
*******************************************************************/
void EPD_HW_RESET(void)
{
  vTaskDelay(100 / portTICK_PERIOD_MS);
  EPD_RES_Clr();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  EPD_RES_Set();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  EPD_READBUSY();
}

/*******************************************************************
    函数说明:更新函数
    入口参数:无  
    说明:更新显示内容到E-Paper    
*******************************************************************/
void EPD_Update(void)
{
  EPD_WR_REG(0x22);
  EPD_WR_DATA8(0xf4);
  EPD_WR_REG(0x20);
  EPD_READBUSY();
}

/*******************************************************************
    函数说明:局刷更新函数
    入口参数:无
    说明:E-Paper工作在局刷模式
*******************************************************************/
void EPD_PartUpdate(void)
{  
  EPD_WR_REG(0x22);
  EPD_WR_DATA8(0xDC);
  EPD_WR_REG(0x20);
  EPD_READBUSY();
}
/*******************************************************************
    函数说明:快刷更新函数
    入口参数:无
    说明:E-Paper工作在快刷模式
*******************************************************************/
void EPD_FastUpdate(void)
{
  EPD_WR_REG(0x22);
  EPD_WR_DATA8(0xC7);
  EPD_WR_REG(0x20);
  EPD_READBUSY();
}


/*******************************************************************
    函数说明:休眠函数
    入口参数:无
    说明:屏幕进入低功耗模式    
*******************************************************************/
void EPD_DeepSleep(void)
{
  EPD_WR_REG(0x10);
  EPD_WR_DATA8(0x01);
  vTaskDelay(200/portTICK_PERIOD_MS);
}

/*******************************************************************
    函数说明:初始化函数
    入口参数:无
    说明:调整E-Paper默认显示方向
*******************************************************************/
void EPD_Init(void)
{
  EPD_HW_RESET();
  EPD_READBUSY();   
  EPD_WR_REG(0x12);  //SWRESET
  EPD_READBUSY();    
}

/*******************************************************************
    函数说明:快刷初始化函数
    入口参数:无
    说明:E-Paper工作在快刷模式1
*******************************************************************/
void EPD_FastMode1Init(void)
{
    EPD_HW_RESET();
    EPD_READBUSY();
    EPD_WR_REG(0x12);  
    EPD_READBUSY();
    EPD_WR_REG(0x18 );
    EPD_WR_DATA8(0x80);
    EPD_WR_REG(0x22 );
    EPD_WR_DATA8(0xB1);
    EPD_WR_REG(0x20 );
    EPD_READBUSY();
    EPD_WR_REG(0x1A );
    EPD_WR_DATA8(0x64);
    EPD_WR_DATA8(0x00);
    EPD_WR_REG(0x22 );
    EPD_WR_DATA8(0x91);
    EPD_WR_REG(0x20 );
    EPD_READBUSY();
}
/*******************************************************************
    函数说明:快刷初始化函数
    入口参数:无
    说明:E-Paper工作在快刷模式2
*******************************************************************/
void EPD_FastMode2Init(void)
{
    EPD_HW_RESET();
    EPD_READBUSY();
    EPD_WR_REG(0x12 );  
    EPD_READBUSY();
    EPD_WR_REG(0x18 );
    EPD_WR_DATA8(0x80);
    EPD_WR_REG(0x22 );
    EPD_WR_DATA8(0xB1);
    EPD_WR_REG(0x20 );
    EPD_READBUSY();
    EPD_WR_REG(0x1A );
    EPD_WR_DATA8(0x5A);
    EPD_WR_DATA8(0x00);
    EPD_WR_REG(0x22 );
    EPD_WR_DATA8(0x91);
    EPD_WR_REG(0x20 );
    EPD_READBUSY();
}

/*******************************************************************
    函数说明:清屏函数
    入口参数:无
    说明:E-Paper刷白屏
*******************************************************************/
void EPD_Display_Clear(void)
{
  u16 i;
  EPD_WR_REG(0x3C);
  EPD_WR_DATA8(0x05);
  EPD_WR_REG(0x24);
  for(i=0;i<2888;i++)
  {
    EPD_WR_DATA8(0xFF);
    // EPD_WR_DATA8(0xFF);
  }  
  EPD_READBUSY();
  EPD_WR_REG(0x26);
  for(i=0;i<2888;i++)
  {
    EPD_WR_DATA8(0x00);
  }  
}

/*******************************************************************
    函数说明:局刷擦除旧数据
    入口参数:无
    说明:E-Paper工作在局刷模式下调用
*******************************************************************/
void EPD_Clear_R26H(void)
{
  u16 i;
  EPD_READBUSY();
  EPD_WR_REG(0x26);
  for(i=0;i<2888;i++)
  {
    EPD_WR_DATA8(0xFF);
  }
}

/*******************************************************************
    函数说明:数组数据更新到E-Paper
    入口参数:无
    说明:
*******************************************************************/
void EPD_Display(const u8 *image)
{
  u16 i,j,Width,Height;
  Width=(EPD_W%8==0)?(EPD_W/8):(EPD_W/8+1);
  Height=EPD_H;
  EPD_WR_REG(0x24);
  for (j=0;j<Height;j++) 
  {
    for (i=0;i<Width;i++) 
    {
      EPD_WR_DATA8(image[i+j*Width]);
    }
  }
}


/*******************************************************************
    函数说明：创建图片缓存数组
    接口说明：*image  要传入的图片数组
               Width  图片宽度
               Heighe 图片长度
               Rotate 屏幕显示方向
               Color  显示颜色
    返回值：  无
*******************************************************************/
void Paint_NewImage(u8 *image,u16 Width,u16 Height,u16 Rotate,u16 Color)
{
  Paint.Image = 0x00;
  Paint.Image = image;
  Paint.color = Color;  
  Paint.widthMemory = Width;
  Paint.heightMemory = Height;  
  Paint.widthByte = (Width % 8 == 0)? (Width / 8 ): (Width / 8 + 1);
  Paint.heightByte = Height;     
  Paint.rotate = Rotate;
  if(Rotate==0||Rotate==180) 
  {
    Paint.width=Height;
    Paint.height=Width;
  } 
  else 
  {
    Paint.width = Width;
    Paint.height = Height;
  }
}       


/*******************************************************************
    函数说明：清空缓冲区 
    接口说明：Color  像素点颜色参数
    返回值：  无
*******************************************************************/
void Paint_Clear(u8 Color)
{
  u16 X,Y;
  u32 Addr;
  for(Y=0;Y<Paint.heightByte;Y++) 
  {
    for(X=0;X<Paint.widthByte;X++) 
    {   
      Addr=X+Y*Paint.widthByte;//8 pixel =  1 byte
      Paint.Image[Addr]=Color;
    }
  }
}


/*******************************************************************
    函数说明：点亮一个像素点
    接口说明：Xpoint 像素点x坐标参数
              Ypoint 像素点Y坐标参数
              Color  像素点颜色参数
    返回值：  无
*******************************************************************/
void Paint_SetPixel(u16 Xpoint,u16 Ypoint,u16 Color)
{
  u16 X, Y;
  u32 Addr;
  u8 Rdata;
    switch(Paint.rotate) 
    {
      case 0:
          X=Xpoint;
          Y=Paint.heightMemory-Ypoint-1;    
          break;
      case 90:
          
          X=Paint.widthMemory-Ypoint-1;
          Y=Paint.heightMemory-Xpoint-1;
          break;
      case 180:
          X=Paint.widthMemory-Xpoint-1;
          Y=Ypoint;
          break;
      case 270:
          X=Ypoint;
          Y=Xpoint;
          break;
        default:
            return;
    }
    Addr=X/8+Y*Paint.widthByte;
    Rdata=Paint.Image[Addr];
    if(Color==BLACK)
    {    
      Paint.Image[Addr]=Rdata&~(0x80>>(X % 8)); //将对应数据位置0
    }
    else
    {
      Paint.Image[Addr]=Rdata|(0x80>>(X % 8));   //将对应数据位置1  
    }
}


/*******************************************************************
    函数说明：划线函数
    接口说明：Xstart 像素x起始坐标参数
              Ystart 像素Y起始坐标参数
              Xend   像素x结束坐标参数
              Yend   像素Y结束坐标参数
              Color  像素点颜色参数
    返回值：  无
*******************************************************************/
void EPD_DrawLine(u16 Xstart,u16 Ystart,u16 Xend,u16 Yend,u16 Color)
{   
  u16 Xpoint, Ypoint;
  int dx, dy;
  int XAddway,YAddway;
  int Esp;
  char Dotted_Len;
  Xpoint = Xstart;
  Ypoint = Ystart;
  dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;
  XAddway = Xstart < Xend ? 1 : -1;
  YAddway = Ystart < Yend ? 1 : -1;
  Esp = dx + dy;
  Dotted_Len = 0;
  for (;;) {
        Dotted_Len++;
            Paint_SetPixel(Xpoint, Ypoint, Color);
        if (2 * Esp >= dy) {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx) {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}
/*******************************************************************
    函数说明：画矩形函数
    接口说明：Xstart 矩形x起始坐标参数
              Ystart 矩形Y起始坐标参数
              Xend   矩形x结束坐标参数
              Yend   矩形Y结束坐标参数
              Color  像素点颜色参数
              mode   矩形是否进行填充
    返回值：  无
*******************************************************************/
void EPD_DrawRectangle(u16 Xstart,u16 Ystart,u16 Xend,u16 Yend,u16 Color,u8 mode)
{
  u16 i;
    if (mode)
      {
        for(i = Ystart; i < Yend; i++) 
        {
          EPD_DrawLine(Xstart,i,Xend,i,Color);
        }
      }
    else 
     {
        EPD_DrawLine(Xstart, Ystart, Xend, Ystart, Color);
        EPD_DrawLine(Xstart, Ystart, Xstart, Yend, Color);
        EPD_DrawLine(Xend, Yend, Xend, Ystart, Color);
        EPD_DrawLine(Xend, Yend, Xstart, Yend, Color);
     }
}
/*******************************************************************
    函数说明：画圆函数
    接口说明：X_Center 圆心x起始坐标参数
              Y_Center 圆心Y坐标参数
              Radius   圆形半径参数
              Color  像素点颜色参数
              mode   圆形是否填充显示
    返回值：  无
*******************************************************************/
void EPD_DrawCircle(u16 X_Center,u16 Y_Center,u16 Radius,u16 Color,u8 mode)
{
  int Esp, sCountY;
  u16 XCurrent, YCurrent;
  XCurrent = 0;
  YCurrent = Radius;
  Esp = 3 - (Radius << 1 );
    if (mode) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                Paint_SetPixel(X_Center + XCurrent, Y_Center + sCountY, Color);//1
                Paint_SetPixel(X_Center - XCurrent, Y_Center + sCountY, Color);//2
                Paint_SetPixel(X_Center - sCountY, Y_Center + XCurrent, Color);//3
                Paint_SetPixel(X_Center - sCountY, Y_Center - XCurrent, Color);//4
                Paint_SetPixel(X_Center - XCurrent, Y_Center - sCountY, Color);//5
                Paint_SetPixel(X_Center + XCurrent, Y_Center - sCountY, Color);//6
                Paint_SetPixel(X_Center + sCountY, Y_Center - XCurrent, Color);//7
                Paint_SetPixel(X_Center + sCountY, Y_Center + XCurrent, Color);
            }
            if ((int)Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } else { //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            Paint_SetPixel(X_Center + XCurrent, Y_Center + YCurrent, Color);//1
            Paint_SetPixel(X_Center - XCurrent, Y_Center + YCurrent, Color);//2
            Paint_SetPixel(X_Center - YCurrent, Y_Center + XCurrent, Color);//3
            Paint_SetPixel(X_Center - YCurrent, Y_Center - XCurrent, Color);//4
            Paint_SetPixel(X_Center - XCurrent, Y_Center - YCurrent, Color);//5
            Paint_SetPixel(X_Center + XCurrent, Y_Center - YCurrent, Color);//6
            Paint_SetPixel(X_Center + YCurrent, Y_Center - XCurrent, Color);//7
            Paint_SetPixel(X_Center + YCurrent, Y_Center + XCurrent, Color);//0
            if ((int)Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}

/******************************************************************************
      函数说明：显示汉字串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                sizey 字号 
                color 文字颜色
      返回值：  无
******************************************************************************/
// void EPD_ShowChinese(u16 x,u16 y,u8 *s,u8 sizey,u16 color)
// {
//   while(*s!=0)
//   {
//     if(sizey==12) EPD_ShowChinese12x12(x,y,s,sizey,color);
//     else if(sizey==16) EPD_ShowChinese16x16(x,y,s,sizey,color);
//     else if(sizey==24) EPD_ShowChinese24x24(x,y,s,sizey,color);
//     else if(sizey==32) EPD_ShowChinese32x32(x,y,s,sizey,color);
//     else return;
//     s+=2;
//     x+=sizey;
//   }
// }

/******************************************************************************
      函数说明：显示单个12x12汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                sizey 字号
                color 文字颜色
      返回值：  无
******************************************************************************/
// void EPD_ShowChinese12x12(u16 x,u16 y,u8 *s,u8 sizey,u16 color)
// {
//   u8 i,j;
//   u16 k;
//   u16 HZnum;//汉字数目
//   u16 TypefaceNum;//一个字符所占字节大小
//   u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;                    
//   HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);  //统计汉字数目
//   for(k=0;k<HZnum;k++) 
//   {
//     if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
//     {   
//       for(i=0;i<TypefaceNum;i++)
//       {
//         for(j=0;j<8;j++)
//         {  
//             if(tfont12[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);//画一个点
//             x++;
//             if((x-x0)==sizey)
//             {
//               x=x0;
//               y++;
//               break;
//             }
//         }
//       }
//     }            
//     continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//   }
// } 

/******************************************************************************
      函数说明：显示单个16x16汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                sizey 字号
                color 文字颜色
      返回值：  无
******************************************************************************/
// void EPD_ShowChinese16x16(u16 x,u16 y,u8 *s,u8 sizey,u16 color)
// {
//   u8 i,j;
//   u16 k;
//   u16 HZnum;//汉字数目
//   u16 TypefaceNum;//一个字符所占字节大小
//   u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//   HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);  //统计汉字数目
//   for(k=0;k<HZnum;k++) 
//   {
//     if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
//     {   
//       for(i=0;i<TypefaceNum;i++)
//       {
//         for(j=0;j<8;j++)
//         {  
//             if(tfont16[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);//画一个点
//             x++;
//             if((x-x0)==sizey)
//             {
//               x=x0;
//               y++;
//               break;
//             }
//         }
//       }
//     }            
//     continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//   }
// } 


/******************************************************************************
      函数说明：显示单个24x24汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                sizey 字号
                color 文字颜色
      返回值：  无
******************************************************************************/
// void EPD_ShowChinese24x24(u16 x,u16 y,u8 *s,u8 sizey,u16 color)
// {
//   u8 i,j;
//   u16 k;
//   u16 HZnum;//汉字数目
//   u16 TypefaceNum;//一个字符所占字节大小
//   u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//   HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);  //统计汉字数目
//   for(k=0;k<HZnum;k++) 
//   {
//     if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
//     {   
//       for(i=0;i<TypefaceNum;i++)
//       {
//         for(j=0;j<8;j++)
//         {  
//           if(tfont24[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);//画一个点
//           x++;
//           if((x-x0)==sizey)
//           {
//             x=x0;
//             y++;
//             break;
//           }
//         }
//       }
//     }            
//     continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//   }
// } 

/******************************************************************************
      函数说明：显示单个32x32汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                sizey 字号
                color 文字颜色
      返回值：  无
******************************************************************************/
// void EPD_ShowChinese32x32(u16 x,u16 y,u8 *s,u8 sizey,u16 color)
// {
//   u8 i,j;
//   u16 k;
//   u16 HZnum;//汉字数目
//   u16 TypefaceNum;//一个字符所占字节大小
//   u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//   HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);  //统计汉字数目
//   for(k=0;k<HZnum;k++) 
//   {
//     if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
//     {   
//       for(i=0;i<TypefaceNum;i++)
//       {
//         for(j=0;j<8;j++)
//         {  
//             if(tfont32[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);//画一个点
//             x++;
//             if((x-x0)==sizey)
//             {
//               x=x0;
//               y++;
//               break;
//             }
//         }
//       }
//     }            
//     continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//   }
// }


/*******************************************************************
    函数说明：显示单个字符
    接口说明：x      字符x坐标参数
              y      字符Y坐标参数
              chr    要显示的字符
              size1  显示字符字号大小
              Color  像素点颜色参数
    返回值：  无
*******************************************************************/
void EPD_ShowChar(u16 x,u16 y,u16 chr,u16 size1,u16 color)
{
  u16 i,m,temp,size2,chr1;
  u16 x0,y0;
  x0=x,y0=y;
  if(size1==8)size2=6;
  else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
  chr1=chr-' ';  //计算偏移后的值
  for(i=0;i<size2;i++)
  {
    if(size1==8)
        {temp=asc2_0806[chr1][i];} //调用0806字体
    else if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字体
    else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字体
    else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字体
    else if(size1==48)
        {temp=asc2_4824[chr1][i];} //调用2412字体
    else return;
    for(m=0;m<8;m++)
    {
      if(temp&0x01)Paint_SetPixel(x,y,color);
      else Paint_SetPixel(x,y,!color);
      temp>>=1;
      y++;
    }
    x++;
    if((size1!=8)&&((x-x0)==size1/2))
    {x=x0;y0=y0+8;}
    y=y0;
  }
}

/*******************************************************************
    函数说明：显示字符串
    接口说明：x      字符串x坐标参数
              y      字符串Y坐标参数
              *chr    要显示的字符串
              size1  显示字符串字号大小
              Color  像素点颜色参数
    返回值：  无
*******************************************************************/
void EPD_ShowString(u16 x,u16 y,u8 *chr,u16 size1,u16 color)
{
  while(*chr!='\0')//判断是不是非法字符!
  {
    EPD_ShowChar(x,y,*chr,size1,color);
    chr++;
    x+=size1/2;
  }
}
/*******************************************************************
    函数说明：指数运算
    接口说明：m 底数
              n 指数
    返回值：  m的n次方
*******************************************************************/
u32 EPD_Pow(u16 m,u16 n)
{
  u32 result=1;
  while(n--)
  {
    result*=m;
  }
  return result;
}
/*******************************************************************
    函数说明：显示整型数字
    接口说明：x      数字x坐标参数
              y      数字Y坐标参数
              num    要显示的数字
              len    数字的位数
              size1  显示字符串字号大小
              Color  像素点颜色参数
    返回值：  无
*******************************************************************/
void EPD_ShowNum(u16 x,u16 y,u32 num,u16 len,u16 size1,u16 color)
{
  u8 t,temp,m=0;
  if(size1==8)m=2;
  for(t=0;t<len;t++)
  {
    temp=(num/EPD_Pow(10,len-t-1))%10;
      if(temp==0)
      {
        EPD_ShowChar(x+(size1/2+m)*t,y,'0',size1,color);
      }
      else 
      {
        EPD_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,color);
      }
  }
}
/*******************************************************************
    函数说明：显示浮点型数字
    接口说明：x      数字x坐标参数
              y      数字Y坐标参数
              num    要显示的浮点数
              len    数字的位数
              pre    浮点数的精度
              size1  显示字符串字号大小
              Color  像素点颜色参数
    返回值：  无
*******************************************************************/

void EPD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u8 pre,u8 sizey,u8 color)
{           
  u8 t,temp,sizex;
  u16 num1;
  sizex=sizey/2;
  num1=num*EPD_Pow(10,pre);
  for(t=0;t<len;t++)
  {
    temp=(num1/EPD_Pow(10,len-t-1))%10;
    if(t==(len-pre))
    {
      EPD_ShowChar(x+(len-pre)*sizex,y,'.',sizey,color);
      t++;
      len+=1;
    }
     EPD_ShowChar(x+t*sizex,y,temp+48,sizey,color);
  }
}





//GUI显示秒表
void EPD_ShowWatch(u16 x,u16 y,float num,u8 len,u8 pre,u8 sizey,u8 color)
{           
  u8 t,temp,sizex;
  u16 num1;
  sizex=sizey/2;
  num1=num*EPD_Pow(10,pre);
  for(t=0;t<len;t++)
  {
    temp=(num1/EPD_Pow(10,len-t-1))%10;
    if(t==(len-pre))
    {
      EPD_ShowChar(x+(len-pre)*sizex+(sizex/2-2),y-6,':',sizey,color);
      t++;
      len+=1;
    }
     EPD_ShowChar(x+t*sizex,y,temp+48,sizey,color);
  }
}



void EPD_ShowPicture(u16 x,u16 y,u16 sizex,u16 sizey,const u8 BMP[],u16 Color)
{
  u16 j=0,t;
  u16 i,temp,y0,TypefaceNum=sizex*(sizey/8+((sizey%8)?1:0));
  y0=y;
  for(i=0;i<TypefaceNum;i++)
  {
    temp=BMP[j];
    for(t=0;t<8;t++)
    {
     if(temp&0x80)
     {
       Paint_SetPixel(x,y,Color);
     }
     else
     {
       Paint_SetPixel(x,y,!Color);
     }
     y++;
     temp<<=1;
    }
    if((y-y0)==sizey)
    {
      y=y0;
      x++;
    }
    j++;
  }
}




void epaper_init_gpio(void)
{

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << EXAMPLE_PIN_NUM_EPD_DC) | (1ULL << EXAMPLE_PIN_NUM_EPD_RST),
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << EXAMPLE_PIN_NUM_EPD_BUSY);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

void epaper_spi_init(void)
{
    ESP_LOGI(TAG, "Initializing SPI");
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 16 * 320 * 2 + 8
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = EPD_PANEL_SPI_CLK,    //Clock out at 1 MHz
        .mode = 0,                              //SPI mode 0
        .spics_io_num = EXAMPLE_PIN_NUM_EPD_CS,             //CS pin
        .queue_size = 7,                        //We want to be able to queue 7 transactions at a time
        // .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(EPAPER_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(EPAPER_SPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG,"Initializing EPD GPIO");
    epaper_init_gpio();
    u8 ImageBW[2888];
    Paint_NewImage(ImageBW,EPD_W,EPD_H,180,WHITE);    //创建画布
    Paint_Clear(WHITE);  
    EPD_Init();
    EPD_ShowPicture(0,88,32,32,gImage_temp,BLACK);
    EPD_ShowPicture(0,120,32,32,gImage_himi,BLACK);
    // EPD_ShowString(0,58,(unsigned char *)"hello",16,BLACK);

    EPD_Display(ImageBW);
    Paint_Clear(WHITE);
    EPD_Update();
    EPD_DeepSleep();
}
