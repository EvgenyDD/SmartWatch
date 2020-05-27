/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BMP180_H
#define BMP180_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define BMP180_ADDRESS  (0x77<<1)

typedef enum {
    BMP180_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
    BMP180_REGISTER_CHIPID             = 0xD0,
    BMP180_REGISTER_VERSION            = 0xD1,
    BMP180_REGISTER_SOFTRESET          = 0xE0,
    BMP180_REGISTER_CONTROL            = 0xF4,
    BMP180_REGISTER_TEMPDATA           = 0xF6,
    BMP180_REGISTER_PRESSUREDATA       = 0xF6,
    BMP180_CMD_READTEMP                = 0x2E,
    BMP180_CMD_READPRESSURE            = 0x34
} BMP180_REG_t;

typedef enum{
      BMP180_MODE_ULTRALOWPOWER          = 0,
      BMP180_MODE_STANDARD               = 1,
      BMP180_MODE_HIGHRES                = 2,
      BMP180_MODE_ULTRAHIGHRES           = 3
} BMP180_mode_t;

typedef struct{
  int16_t  ac1;
  int16_t  ac2;
  int16_t  ac3;
  uint16_t ac4;
  uint16_t ac5;
  uint16_t ac6;
  int16_t  b1;
  int16_t  b2;
  int16_t  mb;
  int16_t  mc;
  int16_t  md;
} BMP180_calib_data;
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
bool BMP180_Init(void);

bool BMP180_Read(BMP180_REG_t, uint8_t*, uint8_t);
void BMP180_Read16(BMP180_REG_t, uint16_t *dest);
void BMP180_Read16S(BMP180_REG_t, int16_t *dest);
bool BMP180_Write(uint8_t reg_addr, uint8_t value);

uint16_t BMP180_GetPressure(void);
void readCoefficients(void);


    bool  begin(void);
    void  getTemperature(float *temp);
    void  getPressure(float *pressure);
    float pressureToAltitude(float seaLvel, float atmospheric);
    float seaLevelForAltitude(float altitude, float atmospheric);
    // Note that the next two functions are just for compatibility with old
    // code that passed the temperature as a third parameter.  A newer
    // calculation is used which does not need temperature.
    float pressureToAltitude2(float seaLevel, float atmospheric, float temp);
    float seaLevelForAltitude2(float altitude, float atmospheric, float temp);

    int32_t computeB5(int32_t ut);


#endif //BMP180_H
