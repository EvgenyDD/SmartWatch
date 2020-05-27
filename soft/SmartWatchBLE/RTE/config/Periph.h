/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PERIPH_H
#define PERIPH_H

/* Includes ------------------------------------------------------------------*/
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_pwm.h"
#include "twi_master.h"

#include "hardware.h"

#include "SSD1306.h"
#include "LSM303.h"
#include "BMP180.h"
#include "light.h"
#include "sound.h"

#include "timework.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define TWI_ENABLE 	NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
#define TWI_DISABLE	NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;

#define SPI_ENABLE 	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos;
#define SPI_DISABLE	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos;

#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (1<<(m)))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void InitPeriph(void);
void LFCLK_Config(void);
void RTC1_Config(void);


#endif //PERIPH_H
