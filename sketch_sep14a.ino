/******************************************************************************
-------------------------1.开发环境:Arduino IDE-----------------------------------
-------------------------------2.作者：starky-----------------------------------------
DS1302:from https://www.jianshu.com/p/142435a024a1
******************************************************************************/
#include <Arduino.h>
#include "DS1302.h"

#include <Wire.h>
#include "AHT10.h"

#include "Arduino.h" //用于包含如ADMUX等寄存器的宏
//code from:https://blog.csdn.net/u013810296/article/details/86746666
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#define ADMUX_VCCWRT1V1 (_BV(MUX5) | _BV(MUX0))
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define ADMUX_VCCWRT1V1 (_BV(MUX3) | _BV(MUX2))
#else
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
#endif


float Read_Volts1(void)
{
    int sensorValue = analogRead(0);
    // 将模拟信号转换成电压
    float voltage = sensorValue * (5.0 / 1023.0);
    return voltage;
}


float Read_Volts(void)
{
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
    if (ADMUX != ADMUX_VCCWRT1V1)
    {
        ADMUX = ADMUX_VCCWRT1V1;

        // Bandgap reference start-up time: max 70us
        // Wait for Vref to settle.
        delayMicroseconds(350);
    }

    // Start conversion and wait for it to finish.
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC)) {};

    // Result is now stored in ADC.

    // Calculate Vcc (in V)
    float vcc = 1.1 * 1024.0 / ADC;

    return vcc;
}

//arduino uno r3 引脚接法

uint8_t din = 8; // DA
uint8_t clk = 7; // CK
uint8_t cs = 6; // CS
uint8_t pk = 2; // power key
uint8_t Reset = 1; // RS
uint8_t en = 0; // EN
uint8_t CTL1 = 3;  //mode
uint8_t CTL2 = 6;  //set
uint8_t Light = 1;  //set

uint8_t dim = 50;  //dim
uint8_t last_dim = 100;  //dim

boolean switch_state = true;
int tomato_time = 1500;
int Light_Offenset = 0;



int h1 = 0;
int h2 = 0;
int m1 = 0;
int m2 = 0;
int s1 = 0;
int s2 = 0;

void write_6302(unsigned char w_data)
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        digitalWrite(clk, LOW);
        if ((w_data & 0x01) == 0x01)
        {
            digitalWrite(din, HIGH);
        }
        else
        {
            digitalWrite(din, LOW);
        }
        w_data >>= 1;
        digitalWrite(clk, HIGH);
    }
}

void VFD_cmd(unsigned char command)
{
    digitalWrite(cs, LOW);
    write_6302(command);
    digitalWrite(cs, HIGH);
    delayMicroseconds(1);
}

void S1201_show(void)
{
    digitalWrite(cs, LOW);//开始传输
    write_6302(0xe8);     //地址寄存器起始位置
    digitalWrite(cs, HIGH); //停止传输
}

void VFD_init()
{
    //SET HOW MANY digtal numbers
    digitalWrite(cs, LOW);
    write_6302(0xe0);
    delayMicroseconds(5);
    write_6302(0x07);//8 digtal
    digitalWrite(cs, HIGH);
    delayMicroseconds(5);

    //set bright
    digitalWrite(cs, LOW);
    write_6302(0xe4);
    delayMicroseconds(5);
    write_6302(0x0f);//leve 255 max
    digitalWrite(cs, HIGH);
    delayMicroseconds(5);
}

void VFD_dim(unsigned char dim)
{
    //set bright
    digitalWrite(cs, LOW);
    write_6302(0xe4);
    delayMicroseconds(5);
    write_6302(dim);//leve 255 max
    digitalWrite(cs, HIGH);
    delayMicroseconds(5);
}

/******************************
  在指定位置打印一个number(用户自定义,所有CG-ROM中的)
  x:0~11;chr:要显示的字符编码
*******************************/
void S1201_WriteOneChar(unsigned char x, unsigned char chr)
{
    digitalWrite(cs, LOW);  //开始传输
    write_6302(0x20 + x); //地址寄存器起始位置
    write_6302(chr + 0x30);
    digitalWrite(cs, HIGH); //停止传输
    S1201_show();
}


/******************************
  在指定位置打印一个字符(用户自定义,所有CG-ROM中的)
  x:0~11;chr:要显示的字符编码
*******************************/
void S1201_WriteOneChar1(unsigned char x, unsigned char chr)
{
    digitalWrite(cs, LOW);  //开始传输
    write_6302(0x20 + x); //地址寄存器起始位置
    write_6302(chr);
    digitalWrite(cs, HIGH); //停止传输
    S1201_show();
}

/******************************
  在指定位置打印字符串
  (仅适用于英文,标点,数字)
  x:0~11;str:要显示的字符串
*******************************/
void S1201_WriteStr(unsigned char x, char* str)
{
    digitalWrite(cs, LOW);  //开始传输
    write_6302(0x20 + x); //地址寄存器起始位置
    while (*str)
    {
        write_6302(*str); //ascii与对应字符表转换
        str++;
    }
    digitalWrite(cs, HIGH); //停止传输
    S1201_show();
}


void S1201_WriteNumShade(unsigned char x, unsigned char chr, int delaytime)
{
    int slot = 10;
    int times = delaytime / slot;
    int i = 0;
    for (i = 0; i < times; i++)
    {
        S1201_WriteOneChar(x, chr);
        delay(slot);
        VFD_dim((255 * (delaytime - i * slot)) / delaytime);
    }
}

void S1201_WriteNumLight(unsigned char x, unsigned char chr, int delaytime)
{
    int slot = 10;
    int times = delaytime / slot;
    int i = 0;
    for (i = times; i > 0; i++)
    {
        S1201_WriteOneChar(x, chr);
        delay(slot);
        VFD_dim((255 * (i * slot)) / delaytime);
    }
}


void Zero_time()
{
    h1 = -1;
    h2 = -1;
    m1 = -1;
    m2 = -1;
    s1 = -1;
    s2 = -1;
}

//显示时间
void Display_RTCC()
{

    if (DS1302Buffer.Second == 57)//57秒的时候显示年月日
    {
        if (DS1302Buffer.Year >= 10)
        {
            int year1 = DS1302Buffer.Year / 10;
            int year2 = DS1302Buffer.Year % 10;
            S1201_WriteOneChar(0, year1);
            S1201_WriteOneChar(1, year2);

        }
        S1201_WriteOneChar1(2, '-');
        int mon1 = DS1302Buffer.Month / 10;
        int mon2 = DS1302Buffer.Month % 10;
        S1201_WriteOneChar(3, mon1);
        S1201_WriteOneChar(4, mon2);
        S1201_WriteOneChar1(5, '-');
        int day1 = DS1302Buffer.Day / 10;
        int day2 = DS1302Buffer.Day % 10;
        S1201_WriteOneChar(6, day1);
        S1201_WriteOneChar(7, day2);
        delay(1000);
        Zero_time();

    }
    else if (DS1302Buffer.Second == 58)//58的时候显示星期
    {
        switch (DS1302Buffer.Week)
        {
        case 1:
            S1201_WriteStr(0, "Monday:)");         //显示星期一
            break;
        case 2:
            S1201_WriteStr(0, "TUESDAY2");          //显示星期二
            break;
        case 3:
            S1201_WriteStr(0, "WEDNESDY");          //显示星期三
            break;
        case 4:
            S1201_WriteStr(0, "THURSDAY");          //显示星期四
            break;
        case 5:
            S1201_WriteStr(0, "FRIDAY:)");          //显示星期五
            break;
        case 6:
            S1201_WriteStr(0, "STAURDAY");          //显示星期六
            break;
        case 7:
            S1201_WriteStr(0, "SUNDAY:)");          //显示星期日
            break;
        default: break;
        }
        delay(1000);
        Zero_time();

    }
    else
    {
        //int slot = 10;
        //int half_times = 500/10;

        int hour1 = DS1302Buffer.Hour / 10;
        int hour2 = DS1302Buffer.Hour % 10;
        if (hour1 != h1)
            S1201_WriteOneChar(0, hour1);
        if (hour2 != h2)
            S1201_WriteOneChar(1, hour2);

        S1201_WriteOneChar1(2, ':');
        S1201_WriteOneChar1(5, ':');

        int mins1 = DS1302Buffer.Minute / 10;
        int mins2 = DS1302Buffer.Minute % 10;
        if (mins1 != m1)
            S1201_WriteOneChar(3, mins1);
        if (mins2 != m2)
            S1201_WriteOneChar(4, mins2);


        int sec1 = DS1302Buffer.Second / 10;
        int sec2 = DS1302Buffer.Second % 10;
        if (sec1 != s1)
            S1201_WriteOneChar(6, sec1);
        if (sec2 != s2)
            S1201_WriteOneChar(7, sec2);
        delay(500);
        S1201_WriteOneChar1(2, ' ');
        S1201_WriteOneChar1(5, ' ');
        delay(500);

        h1 = hour1;
        h2 = hour1;
        m1 = mins1;
        m2 = mins2;
        s1 = sec1;
        s2 = sec2;

    }


}


//开关是否按下函数
boolean press_switch()
{

    boolean now_state;
    int sensorValue = analogRead(CTL1);
    if (sensorValue > 800) now_state = true;
    else now_state = false;

    if (now_state != switch_state)
    {
        switch_state = now_state;
        return true;
    }
    else
        return false;
}



//时间设置函数
void Set_Time1(int year, int month, int day, int week, int hour, int mins, int secs)
{
    DS1302_ON_OFF(0);     //关闭振荡
    DS1302Buffer.Year = year;
    DS1302Buffer.Month = month;
    DS1302Buffer.Day = day;
    DS1302Buffer.Week = week;
    DS1302Buffer.Hour = hour;
    DS1302Buffer.Minute = mins;
    DS1302Buffer.Second = secs;
    DS1302_SetTime(DS1302_YEAR, DS1302Buffer.Year);
    DS1302_SetTime(DS1302_MONTH, DS1302Buffer.Month);
    DS1302_SetTime(DS1302_DAY, DS1302Buffer.Day);
    DS1302_SetTime(DS1302_WEEK, DS1302Buffer.Week);
    DS1302_SetTime(DS1302_HOUR, DS1302Buffer.Hour);
    DS1302_SetTime(DS1302_MINUTE, DS1302Buffer.Minute);
    DS1302_SetTime(DS1302_SECOND, DS1302Buffer.Second);
    DS1302_ON_OFF(1);


    Display_RTCC();
}


void tomato(int i)
{
    int j = i;
    boolean c = press_switch();
    while (c == false && j > 0)
    {
        print_tomato_time(j);
        j -= 1;
        c = press_switch();
    }
    if (c == true) return;

    //未确认状态
    int e = 0;
    VFD_dim(253);
    while (press_switch() == false)
    {
        
        switch (e)
        {
        case 0: S1201_WriteStr(0, "->    <-"); break;
        case 1: S1201_WriteStr(0, " ->  <- "); break;
        case 2: S1201_WriteStr(0, "  -><-  "); break;
        case 3: S1201_WriteStr(0, "  <-->  "); break;
        case 4: S1201_WriteStr(0, " <-  -> "); break;
        case 5: S1201_WriteStr(0, "<-    ->"); break;
        case 6: S1201_WriteStr(0, "        "); break;
        case 7: S1201_WriteStr(0, "->    <-"); break;
        case 8: S1201_WriteStr(0, "        "); break;


        default:
            S1201_WriteStr(0, "->    <-");
            delay(100);
            S1201_WriteStr(0, "<-    ->");
            delay(100);
            break;
        }
        e++;
        delay(180);
    }

    VFD_dim(dim);

    take_rest();
    S1201_WriteStr(0, "Go on ??");
    delay(3000);

    if (press_switch() == true)
    {
        tomato(tomato_time);
    }



}


//番茄时间打印函数
void print_tomato_time(int i)
{
    int mins = (i % 3600) / 60;
    int mins1 = mins / 10;
    int mins2 = mins % 10;
    int sec = (i % 3600) % 60;
    int sec1 = sec / 10;
    int sec2 = sec % 10;
    S1201_WriteOneChar1(0, 'R');
    S1201_WriteOneChar1(1, 'M');

    S1201_WriteOneChar(3, mins1);
    S1201_WriteOneChar(4, mins2);
    S1201_WriteOneChar(6, sec1);
    S1201_WriteOneChar(7, sec2);

    S1201_WriteOneChar1(2, ':');
    S1201_WriteOneChar1(5, '-');
    delay(500);


    S1201_WriteOneChar1(2, ' ');
    S1201_WriteOneChar1(5, ' ');


    delay(500);




}
//番茄休息时间
void take_rest()
{
    int slot = 220;
    S1201_WriteStr(0, ":-) TAKE");
    delay(slot);
    S1201_WriteStr(0, ":-) AKE ");
    delay(slot);
    S1201_WriteStr(0, ":-) KE A");
    delay(slot);
    S1201_WriteStr(0, ":-) E A R");
    delay(slot);
    S1201_WriteStr(0, ":-)  A RE");
    delay(slot);
    S1201_WriteStr(0, ":-) A RES");
    delay(slot);
    S1201_WriteStr(0, ":-) REST");
    delay(slot);
    int i = 10;
    while ((i >= 0) && press_switch() == false)
    {

        int mins = (i % 3600) / 60;
        int mins1 = mins / 10;
        int mins2 = mins % 10;
        int sec = (i % 3600) % 60;
        int sec1 = sec / 10;
        int sec2 = sec % 10;

        S1201_WriteOneChar1(0, ':');
        S1201_WriteOneChar1(1, '-');
        S1201_WriteOneChar1(2, 'D');
        S1201_WriteOneChar1(3, ' ');
        S1201_WriteOneChar(4, mins1);
        S1201_WriteOneChar(5, mins2);
        S1201_WriteOneChar(6, sec1);
        S1201_WriteOneChar(7, sec2);

        delay(500);


        S1201_WriteOneChar1(0, ':');
        S1201_WriteOneChar1(1, '-');
        S1201_WriteOneChar1(2, ')');
        delay(500);
        i -= 1;
    }
}


//打印函数
void print_tomato()
{
    int slot = 170;


    S1201_WriteStr(0, " TOMATO ");
    delay(slot);
    S1201_WriteStr(0, "        ");
    delay(slot);

}
void print_bye()
{
    int slot = 300;
    S1201_WriteStr(0, "---BYE--");
    delay(slot);
    S1201_WriteStr(0, "        ");
    delay(slot);
    S1201_WriteStr(0, "---BYE--");
    delay(slot);
    S1201_WriteStr(0, "        ");
    delay(slot);


}



void print_vcc()
{
    float vcc = Read_Volts1();
    int v = int(vcc * 100);//398
    int v1 = v / 100;//3
    int v2 = (v - v1 * 100) / 10;//9
    int v3 = (v - v1 * 100) % 10;//8
    S1201_WriteOneChar1(0, 'V');
    S1201_WriteOneChar1(1, 'C');
    S1201_WriteOneChar1(2, 'C');
    S1201_WriteOneChar1(3, ':');
    S1201_WriteOneChar(4, v1);
    S1201_WriteOneChar1(5, '.');
    S1201_WriteOneChar(6, v2);
    S1201_WriteOneChar(7, v3);

}

void print_bat()
{
    float vcc = Read_Volts1();
    int v = int(vcc * 100);//398

    S1201_WriteOneChar1(0, 'B');
    S1201_WriteOneChar1(1, 'a');
    S1201_WriteOneChar1(2, 't');
    S1201_WriteOneChar1(3, ':');
    S1201_WriteOneChar1(4, ' ');
    S1201_WriteOneChar1(7, '%');

    if (v > 420)
    {
        S1201_WriteOneChar(4, 1);
        S1201_WriteOneChar(5, 0);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 406)
    {
        S1201_WriteOneChar(5, 9);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 398)
    {
        S1201_WriteOneChar(5, 8);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 392)
    {
        S1201_WriteOneChar(5, 7);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 387)
    {
        S1201_WriteOneChar(5, 6);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 382)
    {
        S1201_WriteOneChar(5, 5);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 379)
    {
        S1201_WriteOneChar(5, 4);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 377)
    {
        S1201_WriteOneChar(5, 3);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 374)
    {
        S1201_WriteOneChar(5, 2);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 368)
    {
        S1201_WriteOneChar(5, 1);
        S1201_WriteOneChar(6, 0);
    }
    else if (v > 345)
    {
        S1201_WriteOneChar(5, 0);
        S1201_WriteOneChar(6, 5);
    }

    else
    {
        S1201_WriteOneChar(5, 0);
        S1201_WriteOneChar(6, 0);
    }


}

void print_dim()
{

    int i = (100 * dim) / 255;
    int i1 = i / 100;//3
    int i2 = (i - i1 * 100) / 10;//9
    int i3 = (i - i1 * 100) % 10;//8
    S1201_WriteOneChar1(0, 'D');
    S1201_WriteOneChar1(1, 'i');
    S1201_WriteOneChar1(2, 'm');
    S1201_WriteOneChar1(3, ':');
    if (i1 != 0)
        S1201_WriteOneChar(4, i1);
    else
        S1201_WriteOneChar1(4, ' ');

    S1201_WriteOneChar(5, i2);

    S1201_WriteOneChar(6, i3);

    S1201_WriteOneChar1(7, '%');

}


void print_temp() {
    float temp, hum;
    AHT10::measure(&temp, &hum);
    int t = int(temp * 10);//289
    int t1 = t / 100;//2
    int t2 = (t - t1 * 100) / 10;//9
    int t3 = (t - t1 * 100) % 10;//8
    S1201_WriteOneChar1(0, ' ');
    S1201_WriteOneChar1(1, 'T');
    S1201_WriteOneChar1(2, ':');
    S1201_WriteOneChar(3, t1);
    S1201_WriteOneChar(4, t2);
    S1201_WriteOneChar1(5, '.');
    S1201_WriteOneChar(6, t3);
    S1201_WriteOneChar1(7, 'C');


}
//打印湿度信息
void print_hum() {
    float temp, hum;
    AHT10::measure(&temp, &hum);
    int h = int(hum * 10);//289
    int h1 = h / 100;//2
    int h2 = (h - h1 * 100) / 10;//9
    int h3 = (h - h1 * 100) % 10;//8
    S1201_WriteOneChar1(0, ' ');
    S1201_WriteOneChar1(1, 'H');
    S1201_WriteOneChar1(2, ':');
    S1201_WriteOneChar(3, h1);
    S1201_WriteOneChar(4, h2);
    S1201_WriteOneChar1(5, '.');
    S1201_WriteOneChar(6, h3);
    S1201_WriteOneChar1(7, '%');

}


void Light_auto(int x)
{
    int Light = analogRead(1);
    if (Light > 800)
        dim = 10 + x;
    else if (Light > 800)
        dim = 20 + x;
    else if (Light > 700)
        dim = 30 + x;
    else if (Light > 600)
        dim = 50 + x;
    else if (Light > 500)
        dim = 80 + x;
    else if (Light > 400)
        dim = 110 + x;
    else if (Light > 300)
        dim = 140 + x;
    else if (Light > 200)
        dim = 170 + x;
    else if (Light > 100)
        dim = 200 + x;
    if (dim <= 0) dim = 10;
    else if (dim >= 250) dim = 250;

}




void power_save()
{
    VFD_dim(0);

    digitalWrite(pk, HIGH);
    delay(200);
    digitalWrite(pk, LOW);
    delay(200);
    digitalWrite(pk, HIGH);
    delay(200);
    digitalWrite(pk, LOW);
    delay(200);
    digitalWrite(pk, HIGH);
    delay(200);
}


void AHT_setup()
{
    Wire.begin();
    if (!AHT10::begin()) {
        Serial.println(F("AHT10 not detected!"));
        Serial.flush();
        //ESP.deepSleep(0);
    }
}


void setup()
{
    DS1302_Init();
    AHT_setup();
    DS1302_GetTime(&DS1302Buffer);
    Display_RTCC();
    //Set_Time1(20, 10, 02, 5, 19, 18, 0);
    pinMode(en, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(din, OUTPUT);
    pinMode(cs, OUTPUT);
    pinMode(pk, OUTPUT);
    pinMode(Reset, OUTPUT);


    digitalWrite(pk, HIGH);

    digitalWrite(en, HIGH);
    digitalWrite(Reset, LOW);
    delayMicroseconds(5);
    digitalWrite(Reset, HIGH);
    VFD_init();
    VFD_cmd(0xE9);// 全亮
    VFD_dim(dim);
    switch_state = digitalRead(CTL1);
    Zero_time();
}

int i = 0;


void loop()
{
    DS1302_GetTime(&DS1302Buffer);        //获取当前RTCC值
    int vcc = int(Read_Volts1() * 100);
    if (vcc < 370)//待机模式
    {
        power_save();
    }
    else if (vcc < 380)//省电模式
    {
        S1201_WriteStr(0, "        ");
        if ((DS1302Buffer.Second >= 57 && DS1302Buffer.Second <= 59) || (DS1302Buffer.Second == 0) || (DS1302Buffer.Second == 1))
        {
            Display_RTCC();
        }
    }
    else//正常模式
    {
        Light_auto(Light_Offenset);
        VFD_dim(dim);

        if (DS1302Buffer.Second % 58 == 0 && DS1302Buffer.Minute % 3 == 0)//每三分钟输出电池信息
        {
            print_bat();
            Zero_time();
        }
        else
        {
            Display_RTCC();
        }

        if (press_switch() == true)//其它模式
        {

            //显示番茄时钟模式
            while (press_switch() == false)
            {
                int Value = analogRead(CTL2);
                print_tomato();
                if (Value > 800)
                {
                    tomato(tomato_time);
                    print_bye();
                    Zero_time();
                }
            }

            


            //显示电压模式
            while (press_switch() == false)
            {
                print_vcc();
                delay(400);
            }

            //显示电量模式
            while (press_switch() == false)
            {
                print_bat();
            }

            //显示温度模式
            while (press_switch() == false)
            {
                print_temp();
            }

            //显示湿度模式
            while (press_switch() == false)
            {
                print_hum();
            }

            //设置亮度+模式
            while (press_switch() == false)
            {
                print_dim();
                int Value = analogRead(CTL2);
                if (Value > 800)
                {
                    Light_Offenset += 10;
                }
                Light_auto(Light_Offenset);
                VFD_dim(dim);

                delay(200);
            }

            //设置亮度-模式
            while (press_switch() == false)
            {
                print_dim();
                int Value = analogRead(CTL2);
                if (Value > 800)
                {
                    Light_Offenset -= 10;
                }
                Light_auto(Light_Offenset);
                VFD_dim(dim);

                delay(200);
            }



            Zero_time();
        }
    }



}
