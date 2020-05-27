/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LIGHT_H
#define LIGHT_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "nrf.h"


/* Exported types ------------------------------------------------------------*/
typedef struct{
    uint16_t result;
    int16_t value;
}ADCDataType;


/* Exported constants --------------------------------------------------------*/
enum{
    ADC_LIGHT   =ADC_CONFIG_PSEL_AnalogInput2,
    ADC_BAT     =ADC_CONFIG_PSEL_AnalogInput5,
};


/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
extern ADCDataType Light, VBat;


/* Exported functions ------------------------------------------------------- */
void ADCInit(uint8_t ADCChannel);

void MeasureVBat();
void MeasureLight();

#endif //LIGHT_H
