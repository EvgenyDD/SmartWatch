/* Includes ------------------------------------------------------------------*/
#include "UI.h"
#include "nrf.h"
#include "SSD1306.h"
#include "light.h"
#include "timework.h"
#include "hardware.h"
#include "menu.h"
#include "LSM303.h"
#include "twi_master.h"


AlarmType Alarmes[10] = {8,0,127,true};
#define TimeDiff(AlarmH, AlarmM, TimeH, TimeM) (((AlarmH*60+AlarmM)-(TimeH*60+TimeM))>0 && ((AlarmH*60+AlarmM)-(TimeH*60+TimeM))<500)?((AlarmH*60+AlarmM)-(TimeH*60+TimeM)):0

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern uint32_t RTCCounter;
extern char month[];
extern uint8_t j;

extern MenuItem*	CurrWorkItem;

    /* System Tick Timer Variable */
extern uint16_t SysTickTimer;

    /* Image resources */
extern const char menu_alarm[];
extern const char menu_stopwatch[];
extern const char menu_timedate[];
extern const char menu_torch[];
extern const char menu_rand[];
extern const char menu_games[];
extern const char menu_settings[];
extern const char menu_txt[];

extern uint16_t xxxx;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
extern volatile uint8_t buttonCounter;
extern volatile uint16_t buttonState;

void MainScreen(uint16_t opCode)
{
    SSD1306_Clear();

    SSD1306_setFont(font3x5);

    MeasureVBat();
    SSD1306_Float(0, 123, (float)(VBat.result*14.2134+34.024/*71/5+34*/)/1000, 2);
    //SSD1306_Float(0, 123, (float)(1.57927*(VBat.result-229.781)), 1);
    //SSD1306_Num(0,123,(1.57927*(VBat.result-229.781)));

    drawFastHLine(0, 128-122, 64, WHITE);

    SSD1306_setFont(font5x8);
    char t[15];
    RTC_TimeDateTypeDef aaa;
    RTC_CntToTimeDate(RTCCounter, &aaa);
    TimeToString(t, &aaa);
    char temp[10];
    temp[0] = t[0];
    temp[1] = t[1];
    temp[2] = '\0';
    SSD1306_BigString(18, 94, temp,3);
    temp[0] = t[3];
    temp[1] = t[4];
    temp[2] = '\0';
    SSD1306_BigString(18, 70, temp,3);
    temp[0] = t[6];
    temp[1] = t[7];
    temp[2] = '\0';
    SSD1306_BigString(24, 50, temp,2);


    char date[7];
    date[6] = '\0';

    for(uint8_t i=0; i<3; i++)
        date[i] = month[aaa.month*3+i];

    date[3] = ' ';
    date[4] = aaa.date/10+0x30;
    date[5] = aaa.date%10+0x30;

    SSD1306_String(16, 36, date);
    SSD1306_Num(20, 26, aaa.year);

    MeasureLight();
    //SSD1306_NumWDesc(0, 0, "L=", Light.result);

    SSD1306_Num(48, 0, j);
//SSD1306_Num(0, 70, xxxx);
//SSD1306_Num(0, 80, buttonCounter);
//SSD1306_Num(0, 90, buttonState);

    uint16_t num, diffH=0, diffM=0;
    RTC_TimeDateTypeDef tempp;
    RTC_CntToTimeDate(RTCCounter, &tempp);
    uint16_t val=9999;
    for(uint8_t i=0; i<10; i++)
    {
        /* if it's left <500 min to Alarm */
        if(val=TimeDiff(Alarmes[i].hour, Alarmes[i].minute, tempp.hour, tempp.minute)>0)
        {
            /* if we found nearest Alarm */
            if(val > (diffH*60+diffM))
            {
                num = i;
                val = (diffH*60+diffM);
                diffH = val/60;
                diffM = val%60;
            }
        }
    }
    if(val != 9999)
    {
        char tm[6];
        tm[0] = diffH/60;
        tm[1] = diffH%60;
        tm[2] = ':';
        tm[3] = diffM/60;
        tm[4] = diffM%60;
        tm[5] = '\0';
        SSD1306_String(0,0,tm);
    }

}










/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
void BinarScreen(uint16_t opCode)
{
    SSD1306_setFont(font5x8);
    char t[15];
    RTC_TimeDateTypeDef aaa;
    RTC_CntToTimeDate(RTCCounter, &aaa);
    TimeToString(t, &aaa);

    char temp[10];
    temp[0] = t[0];
    temp[1] = t[1];
    temp[2] = '\0';
    SSD1306_BigString(0, 70, temp,6);
    temp[0] = t[3];
    temp[1] = t[4];
    temp[2] = '\0';
    SSD1306_BigString(0, 10, temp,6);

}



extern volatile int ACCEL[3], MAGNET[3];

/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
void ExtendScreen(uint16_t opCode)
{
    SSD1306_Num(0, 50, j);
    SSD1306_Num(0, 70, xxxx);


    SSD1306_NumWDesc(0,0,"X:", ACCEL[XAxis]);
    SSD1306_NumWDesc(0,9,"Y:", ACCEL[YAxis]);
    SSD1306_NumWDesc(0,18,"Z:", ACCEL[ZAxis]);

   /* static int k=0;
    if(++k > 1)
    {
        //k=0;

    }
    if(k>2)
        k=0;*/
}




/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
extern MenuItem Settings;
extern MenuItem Random;
extern MenuItem Calendar;
extern MenuItem Alarm;
extern MenuItem Text;
extern MenuItem Game;
extern MenuItem Flashlight;
extern MenuItem Stopwatch;
extern MenuItem SetTime;

void menuScreen(uint16_t opCode)
{
    /* Icon navigation */
    static uint8_t i=0, j=0;
    switch(opCode)
    {
    case BTN_RIGHT:
        if(++j > 1) j=0;
        break;
    case BTN_LEFT:
        if(--j > 1) j=1;
        break;
    case BTN_UP:
        if(++i > 3) i=0;
        break;
    case BTN_DOWN:
        if(--i > 3) i=3;
        break;
    case BTN_MENU:
        if(j==0)
        {
            if(i==0) {MenuChange(&Settings); return;}
            if(i==1) {MenuChange(&Random); return;}
            if(i==2) {MenuChange(&Calendar); return;}
            if(i==3) {MenuChange(&Alarm); return;}
        }
        else
        {
            if(i==0) {MenuChange(&Text); return;}
            if(i==1) {MenuChange(&Game); return;}
            if(i==2) {MenuChange(&Flashlight); return;}
            if(i==3) {MenuChange(&Stopwatch); return;}
        }
    }

    /* Draw Menu Icons */
    SSD1306_Bitmap(0,96,(char*)menu_alarm);
    SSD1306_Bitmap(32,96,(char*)menu_stopwatch);
    SSD1306_Bitmap(0,64,(char*)menu_timedate);
    SSD1306_Bitmap(32,64,(char*)menu_torch);
    SSD1306_Bitmap(0,32,(char*)menu_rand);
    SSD1306_Bitmap(32,32,(char*)menu_games);
    SSD1306_Bitmap(0,0,(char*)menu_settings);
    SSD1306_Bitmap(32,0,(char*)menu_txt);

    /* Flashing selected icon */
    if(SysTickTimer%8<6)
        SSD1306_InvertRect(i*32,j*32,32,32);
//SSD1306_Num(0, 70, xxxx);
}

void SettScreen(uint16_t code)
{
    static uint8_t ptr = 0;
    SSD1306_String(0, 120, "Time");
    SSD1306_String(0, 110, "Sound");
    SSD1306_String(0, 100, "Vibro");
    SSD1306_String(0, 90,  "RESET");
    switch(code)
    {
    case BTN_RIGHT:
        if(ptr==0) MenuChange(&SetTime);
        break;
    case BTN_LEFT:
        //if(--j > 1) j=1;
        break;
    case BTN_DOWN:
        if(++ptr > 3) ptr=0;
        break;
    case BTN_UP:
        if(--ptr > 3) ptr=3;
        break;
    }
    SSD1306_InvertRect(120+ptr*(-10),0,10,63);
}


void TDScreen(uint16_t code)
{
    SSD1306_setFont(font5x8);
    char t[15];
    RTC_TimeDateTypeDef aaa;
    RTC_CntToTimeDate(RTCCounter, &aaa);
    TimeToString(t, &aaa);


    static uint8_t ptr = 0;
    switch(code)
    {
    case BTN_RIGHT:
        if(ptr == 0) aaa.hour --;
        if(ptr == 1) aaa.minute --;
        if(ptr == 2) aaa.second = 0;
        if(ptr == 3) aaa.date --;
        if(ptr == 4) aaa.month --;
        if(ptr == 5) aaa.year --;
        break;
    case BTN_LEFT:
        if(ptr == 0) aaa.hour ++;
        if(ptr == 1) aaa.minute ++;
        if(ptr == 2) aaa.second = 30;
        if(ptr == 3) aaa.date ++;
        if(ptr == 4) aaa.month ++;
        if(ptr == 5) aaa.year ++;
        break;
    case BTN_DOWN:
        if(++ptr > 5) ptr=0;
        break;
    case BTN_UP:
        if(--ptr > 5) ptr=5;
        break;
    case BTN_MENU:
        MenuChange(PARENT);
        break;
    }

    SSD1306_NumWDesc(0,120,"Hr :", aaa.hour);
    SSD1306_NumWDesc(0,110,"Min:", aaa.minute);
    SSD1306_NumWDesc(0,100,"Sec:", aaa.second);

    char mon[4]= {'\0'};
    for(uint8_t i=0; i<3; i++)
        mon[i] = month[aaa.month*3+i];
    SSD1306_NumWDesc(0,90,"Dat:", aaa.date);
    SSD1306_String(0,80,mon);
    SSD1306_NumWDesc(0,70,"Yr: ",aaa.year);

    SSD1306_InvertRect(120+ptr*(-10),0,10,63);

    RTCCounter = RTC_TimeDateToCnt(&aaa);
}


void SetAlarm(uint16_t code)
{
    static uint8_t ptrX=0, ptrY=0;

    switch(code)
    {
    case BTN_RIGHT:
        if(++ptrX > 11) ptrX=0;
        break;

    case BTN_LEFT:
        if(--ptrX > 11) ptrX=0;
        break;

    case BTN_UP:
        if(--ptrY > 9) ptrY=0;
        break;

    case BTN_DOWN:
        if(++ptrY > 9) ptrY=0;
        break;

    case BTN_MENU:
        if(ptrX==0) Alarmes[ptrY].hour+=10;
        if(ptrX==1) Alarmes[ptrY].hour++;
        if(ptrX==2) Alarmes[ptrY].minute+=10;
        if(ptrX==3) Alarmes[ptrY].minute++;
        if(ptrX == 4) Alarmes[ptrY].active = !Alarmes[ptrY].active;
        if(ptrX > 4) BitFlip(Alarmes[ptrY].day, ptrX-4);
        break;
    }

    SSD1306_setFont(font3x5);
    SSD1306_String(120,31,"MTWTFSS");

    for(uint8_t i=0; i<10; i++)
    {
        char s[15];
        s[0] = Alarmes[i].hour/10+0x30;
        s[1] = Alarmes[i].hour%10+0x30;
        s[2] = ':';
        s[3] = Alarmes[i].minute/10+0x30;
        s[4] = Alarmes[i].minute%10+0x30;
        s[5] = '\0';

        SSD1306_setFont(font5x8);
        SSD1306_String(0, 110+i*(-10), s);

        for(uint8_t j=0; j<8; j++)
        {
            if(j == 0)
                s[0] = Alarmes[i].active?'#':'_';
            else
                s[j] = BitIsSet(Alarmes[i].day, j)?'+':'-';
        }
        s[8] = '\0';

        SSD1306_setFont(font3x5);
        SSD1306_String(31, 110+i*(-10), s);
    }

    const uint8_t constpos[] = {0,6*1,6*3,6*4, 31+0,31+4*1,31+4*2,31+4*3,31+4*4,31+4*5,31+4*6,31+4*7};
    if(ptrX<4)
        SSD1306_InvertRect(110+ptrY*(-10),constpos[ptrX],10,5);
    else
        SSD1306_InvertRect(110+ptrY*(-10),constpos[ptrX],10,3);
}
