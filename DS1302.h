/******************************************************************************
----------------1.开发环境:Arduino IDE-----------------------------------------
----------------2.测试使用开发板型号：Arduino Leonardo or Arduino UNO R3-------
----------------3.单片机使用晶振：16M------------------------------------------
----------------4.作者：神秘藏宝室---------------------------------------------
******************************************************************************/
#ifndef __DS1302_H__
#define __DS1302_H__

#include <Arduino.h>

//DS1302 IO设置
#define DS1302_RST 5     //实时时钟复位线引脚
#define DS1302_IO 4     //实时时钟数据线引脚
#define DS1302_CLK 3     //实时时钟时钟线引脚


#define DS1302_RST_0 digitalWrite(DS1302_RST,LOW)
#define DS1302_RST_1 digitalWrite(DS1302_RST,HIGH)

#define DS1302_IO_0 digitalWrite(DS1302_IO,LOW)
#define DS1302_IO_1 digitalWrite(DS1302_IO,HIGH)

#define DS1302_CLK_0 digitalWrite(DS1302_CLK,LOW)
#define DS1302_CLK_1 digitalWrite(DS1302_CLK,HIGH)


//宏定义
#define DS1302_SECOND   0x80
#define DS1302_MINUTE   0x82
#define DS1302_HOUR     0x84 
#define DS1302_DAY      0x86
#define DS1302_MONTH    0x88
#define DS1302_WEEK     0x8a
#define DS1302_YEAR     0x8c 


typedef struct __SYSTEMTIME__
{
    unsigned char Second;
    unsigned char Minute;
    unsigned char Hour;
    unsigned char Week;
    unsigned char Day;
    unsigned char Month;
    unsigned char  Year;
}SYSTEMTIME;    //定义的时间类型


//函数或者变量声明
extern void DS1302_InputByte(unsigned char dat);
extern unsigned char DS1302_OutputByte(void) ;
extern void DS1302_Write(unsigned char ucAddr, unsigned char ucDa);
extern unsigned char DS1302_Read(unsigned char ucAddr);
extern void DS1302_SetProtect(unsigned char flag) ;
extern void DS1302_SetTime(unsigned char Address, unsigned char Value);
extern void DS1302_GetTime(SYSTEMTIME *Time);
extern void DS1302_Init(void);
extern void DS1302_ON_OFF(bool FLAG_ON_OFF);


extern SYSTEMTIME DS1302Buffer;
extern bool Flag_Time_Refresh;

#endif
