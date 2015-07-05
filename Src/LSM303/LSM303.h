/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LSM303_H
#define LSM303_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>


/* Exported types ------------------------------------------------------------*/
typedef struct{
    int Pitch;
    int Roll;
    int Yaw;
}MAGType;


/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define XAxis 0
#define YAxis 1
#define ZAxis 2

#define LSM303_ADDR_A 0x32
#define LSM303_ADDR_M 0x3C

/* Exported functions ------------------------------------------------------- */
bool LSM303_Init();
bool LSM303_On();
bool LSM303_Off();
bool LSM303_Write(uint8_t reg_addr, uint8_t value);
bool LSM303_Read(uint8_t reg_addr, uint8_t * destination, uint8_t number_of_bytes);


bool LSM303_GetAccelData(int mass[]);
bool LSM303_GetMagnetData(int mass[]);

MAGType SSD1306_CalcHeading();
void getAzimuth();

long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif //LSM303_H
