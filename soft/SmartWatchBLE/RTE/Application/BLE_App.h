/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLE_APP_H
#define BLE_APP_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include "nrf51.h"
#include "softdevice_handler.h"
#include "BLE.h"
#include <stdint.h>
#include <string.h>
#include "nrf_delay.h"
#include "hardware.h"
#include "nrf_gpio.h"

#include "nordic_common.h"
#include "app_error.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_db_discovery.h"
#include "ble_bas.h"
#include "ble_hrs.h"
#include "ble_dis.h"
#include "ble_cts_c.h"

#include "pstorage.h"
#include "device_manager.h"
#include "ble_conn_params.h"
#include "btn_ble.h"

#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif // BLE_DFU_APP_SUPPORT
/* Exported types ------------------------------------------------------------*/
typedef enum
{
	BSP_INDICATE_FIRST = 0, BSP_INDICATE_IDLE = BSP_INDICATE_FIRST, /**< See \ref BSP_INDICATE_IDLE.*/
	BSP_INDICATE_SCANNING, /**< See \ref BSP_INDICATE_SCANNING.*/
	BSP_INDICATE_ADVERTISING, /**< See \ref BSP_INDICATE_ADVERTISING.*/
	BSP_INDICATE_ADVERTISING_WHITELIST, /**< See \ref BSP_INDICATE_ADVERTISING_WHITELIST.*/
	BSP_INDICATE_ADVERTISING_SLOW, /**< See \ref BSP_INDICATE_ADVERTISING_SLOW.*/
	BSP_INDICATE_ADVERTISING_DIRECTED, /**< See \ref BSP_INDICATE_ADVERTISING_DIRECTED.*/
	BSP_INDICATE_BONDING, /**< See \ref BSP_INDICATE_BONDING.*/
	BSP_INDICATE_CONNECTED, /**< See \ref BSP_INDICATE_CONNECTED.*/
	BSP_INDICATE_SENT_OK, /**< See \ref BSP_INDICATE_SENT_OK.*/
	BSP_INDICATE_SEND_ERROR, /**< See \ref BSP_INDICATE_SEND_ERROR.*/
	BSP_INDICATE_RCV_OK, /**< See \ref BSP_INDICATE_RCV_OK.*/
	BSP_INDICATE_RCV_ERROR, /**< See \ref BSP_INDICATE_RCV_ERROR.*/
	BSP_INDICATE_FATAL_ERROR, /**< See \ref BSP_INDICATE_FATAL_ERROR.*/
	BSP_INDICATE_ALERT_0, /**< See \ref BSP_INDICATE_ALERT_0.*/
	BSP_INDICATE_ALERT_1, /**< See \ref BSP_INDICATE_ALERT_1.*/
	BSP_INDICATE_ALERT_2, /**< See \ref BSP_INDICATE_ALERT_2.*/
	BSP_INDICATE_ALERT_3, /**< See \ref BSP_INDICATE_ALERT_3.*/
	BSP_INDICATE_ALERT_OFF, /**< See \ref BSP_INDICATE_ALERT_OFF.*/
	BSP_INDICATE_USER_STATE_OFF, /**< See \ref BSP_INDICATE_USER_STATE_OFF.*/
	BSP_INDICATE_USER_STATE_0, /**< See \ref BSP_INDICATE_USER_STATE_0.*/
	BSP_INDICATE_USER_STATE_1, /**< See \ref BSP_INDICATE_USER_STATE_1.*/
	BSP_INDICATE_USER_STATE_2, /**< See \ref BSP_INDICATE_USER_STATE_2.*/
	BSP_INDICATE_USER_STATE_3, /**< See \ref BSP_INDICATE_USER_STATE_3.*/
	BSP_INDICATE_USER_STATE_ON, /**< See \ref BSP_INDICATE_USER_STATE_ON.*/
	BSP_INDICATE_LAST = BSP_INDICATE_USER_STATE_ON
} bsp_indication_t;

/* Exported constants --------------------------------------------------------*/
#define APP_TIMER_PRESCALER              3276 //+1

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ble_stack_init(void);
void ble_evt_dispatch(ble_evt_t * p_ble_evt);
void sys_evt_dispatch(uint32_t sys_evt);
void db_discovery_init(void);
void on_cts_c_evt(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt);
void current_time_error_handler(uint32_t nrf_error);

void device_manager_init(bool erase_bonds);
void gap_params_init(void);
void advertising_init(void);
void services_init(void);
void conn_params_init(void);

void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
void on_adv_evt(ble_adv_evt_t ble_adv_evt);
uint32_t device_manager_evt_handler(dm_handle_t const * p_handle, dm_event_t const * p_event, ret_code_t event_result);
void conn_params_error_handler(uint32_t nrf_error);

void sleep_mode_enter(void);

void on_ble_evt(ble_evt_t * p_ble_evt);

uint32_t bsp_indication_set(bsp_indication_t indicate);
uint32_t bsp_led_indication(bsp_indication_t indicate);

void advertising_start(void);
void BLE_Stop(void);

void advertising_stop(void);
void app_context_load(dm_handle_t const * p_handle);
void reset_prepare(void);

#endif //BLE_APP_H
