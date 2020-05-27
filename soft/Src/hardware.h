/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef HARDWARE_H
#define HARDWARE_H


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define LED_RED 21UL
#define LED_WHT 24UL
#define LED_YLW 3UL
#define LED_BLU 25UL

#define BTN_0 0UL
#define BTN_1 20UL
#define BTN_2 19UL
#define BTN_3 18UL
#define BTN_4 17UL

#define RAIL_POWER  29UL
#define VIBRO       23UL
#define BUZZER      30UL

#define VIN_SENSE   22UL
#define CHRG_SENSE  5UL
#define CHRG_STBY   6UL

#define AN_LIGHT    1UL
#define AN_MIC      2UL
#define AN_BAT      4UL

#define LSM303_INT1 7UL
#define LSM303_INT2 8UL
#define LSM303_DRDY 16UL

#define BTN_UP      (1<<1)
#define BTN_DOWN    (1<<4)
#define BTN_LEFT    (1<<2)
#define BTN_RIGHT   (1<<3)
#define BTN_MENU    (1<<0)

#define BTN_MENU_LONG   (1<<5)
#define BTN_UP_LONG     (1<<6)
#define BTN_LEFT_LONG   (1<<7)
#define BTN_RIGHT_LONG  (1<<8)
#define BTN_DOWN_LONG   (1<<9)


/* Exported functions ------------------------------------------------------- */


#endif //HARDWARE_H




