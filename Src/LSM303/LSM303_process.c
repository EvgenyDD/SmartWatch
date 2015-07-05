#include "SSD1306.h"
#include "string.h"
#include <math.h>
#include "debug.h"
#include "LSM303.h"


int ACCEL_DATA[3], MAGNET_DATA[3];


#define PI 3.1415

float Pitch=0;
float Roll=0;
float Yaw=0;








float alphaAccel = 0.4;
float alphaMagnet = 0.4;

int xRaw=0;
int yRaw=0;
int zRaw=0;

float xFiltered=0;
float yFiltered=0;
float zFiltered=0;

float xFilteredOld=0;
float yFilteredOld=0;
float zFilteredOld=0;

float xAccel=0;
float yAccel=0;
float zAccel=0;

int xMagnetRaw=0;
int yMagnetRaw=0;
int zMagnetRaw=0;

float xMagnetFiltered=0;
float yMagnetFiltered=0;
float zMagnetFiltered=0;

float xMagnetFilteredOld=0;
float yMagnetFilteredOld=0;
float zMagnetFilteredOld=0;



float xMagnetMap=0;
float yMagnetMap=0;
float zMagnetMap=0;





void FilterAD()
{
    // read from AD and subtract the offset
   /* LSM303_GetAccelData(ACCEL_DATA);
    xRaw = ACCEL_DATA[XAxis] - xOffset;
    yRaw = ACCEL_DATA[YAxis] - yOffset;
    zRaw = ACCEL_DATA[ZAxis] - zOffset;

    xFiltered= xFilteredOld + alphaAccel * (xRaw - xFilteredOld);
    yFiltered= yFilteredOld + alphaAccel * (yRaw - yFilteredOld);
    zFiltered= zFilteredOld + alphaAccel * (zRaw - zFilteredOld);

    xFilteredOld = xFiltered;
    yFilteredOld = yFiltered;
    zFilteredOld = zFiltered;

    //read from Compass
    //Digital Low Pass for Noise Reduction for Magnetic Sensor
    LSM303_GetMagnetData(MAGNET_DATA);
    xMagnetRaw = MAGNET_DATA[0];
    yMagnetRaw = MAGNET_DATA[1];
    zMagnetRaw = MAGNET_DATA[2];

    xMagnetFiltered= xMagnetFilteredOld + alphaMagnet * (xMagnetRaw - xMagnetFilteredOld);
    yMagnetFiltered= yMagnetFilteredOld + alphaMagnet * (yMagnetRaw - yMagnetFilteredOld);
    zMagnetFiltered= zMagnetFilteredOld + alphaMagnet * (zMagnetRaw - zMagnetFilteredOld);

    xMagnetFilteredOld = xMagnetFiltered;
    yMagnetFilteredOld = yMagnetFiltered;
    zMagnetFilteredOld = zMagnetFiltered;*/
}



MAGType SSD1306_CalcHeading()
{
/* Get normalized acceleration vectors */
    LSM303_GetAccelData(ACCEL_DATA);
    xAccel = ACCEL_DATA[XAxis];
    yAccel = ACCEL_DATA[YAxis];
    zAccel = ACCEL_DATA[ZAxis];

    float norm = sqrt(xAccel*xAccel + yAccel*yAccel + zAccel*zAccel);
    xAccel /= norm;
    yAccel /= norm;
    zAccel /= norm;

/* Get normalized magnetic vectors */
    LSM303_GetMagnetData(MAGNET_DATA);
    xMagnetMap = MAGNET_DATA[0];
    yMagnetMap = MAGNET_DATA[1];
    zMagnetMap = MAGNET_DATA[2];

    norm = sqrt(xMagnetMap*xMagnetMap + yMagnetMap*yMagnetMap + zMagnetMap*zMagnetMap);
    xMagnetMap /=norm;
    yMagnetMap /=norm;
    zMagnetMap /=norm;



/* Calculating pitch and roll */
    Pitch = asin(-xAccel);
    Roll = asin(yAccel/cos(Pitch));

/* Calculating heading */
    float Mx2 = xMagnetMap*cos(Pitch) + zMagnetMap*sin(Pitch);
    float My2 = xMagnetMap*sin(Roll)*sin(Pitch)  + yMagnetMap*cos(Roll) - zMagnetMap*sin(Roll)*cos(Pitch);
    float Mz2 = -xMagnetMap*cos(Roll)*sin(Pitch) + yMagnetMap*sin(Roll) + zMagnetMap*cos(Roll)*cos(Pitch);

    if(Mx2>0 && My2>=0)
        Yaw = atan(My2/Mx2)*180/PI;
    if(Mx2<0)
        Yaw = 180 + atan(My2/Mx2)*180/PI;
    if(Mx2>0 && My2<=0)
        Yaw = 360 + atan(My2/Mx2)*180/PI;
    if(Mx2==0 && My2<0)
        Yaw = 90;
    if(Mx2==0 && My2>0)
        Yaw = 270;

/* Output results */
/*
    DebugSendNumWDesc("Pitch: \t", (int)(Pitch*180/PI));
    DebugSendNumWDesc("Roll: \t", (int)(Roll*180/PI));
    DebugSendNumWDesc("Yaw: \t", (int)(Yaw));*/
    MAGType MagData;
    MagData.Pitch = (int)(Pitch*180/PI);
    MagData.Roll = (int)(Roll*180/PI);
    MagData.Yaw = (int)(Yaw);

    return MagData;
}
