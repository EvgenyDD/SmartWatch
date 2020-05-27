/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GAMES_H
#define GAMES_H

/* Includes ------------------------------------------------------------------*/
#include "SSD1306.h"

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "hardware.h"

#include "debug.h"

#include "spi_master.h"
#include "app_util_platform.h"
#include "string.h"
#include "menu.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Game1Intro();
void Game1(uint16_t button);
void Game1End();
void draw();
void game1_start();

#endif //GAMES_H

