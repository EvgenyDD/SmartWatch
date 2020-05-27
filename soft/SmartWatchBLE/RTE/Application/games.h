/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GAMES_H
#define GAMES_H

/* Includes ------------------------------------------------------------------*/
#include "SSD1306.h"

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "hardware.h"

//#include "debug.h"

#include "spi_master.h"
#include "app_util_platform.h"
#include "string.h"
#include "menu.h"
#include "app_error.h"
#include "nrf_drv_rng.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Game1Intro(void);
void Game1(uint16_t button);
void draw(void);
void game1_start(void);

uint8_t SlotSpin(uint16_t code);

#endif //GAMES_H
