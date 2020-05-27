/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "BLE_App.h"
#include "app_timer.h"
#include "UI.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define IS_SRVC_CHANGED_CHARACT_PRESENT  1         	/**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(400, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(650, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                    0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)    

#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define DEVICE_NAME                      "SmartWatch"                               /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "BYtronic"                      			/**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                 300                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       180                                        /**< The advertising timeout in units of seconds. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT); /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
#endif // BLE_DFU_APP_SUPPORT

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static ble_bas_t m_bas; /**< Structure used to identify the battery service. */
static ble_hrs_t m_hrs; /**< Structure used to identify the heart rate service. */

static dm_application_instance_t m_app_handle;
static dm_handle_t m_peer_handle; /**< The peer that is currently connected. */

static ble_db_discovery_t m_ble_db_discovery; /**< Structure used to identify the DB Discovery module. */
static ble_cts_c_t m_cts; /**< Structure to store the data of the current time service. */

static ble_uuid_t m_adv_uuids[] =
{/*{BLE_UUID_HEART_RATE_SERVICE,         BLE_UUID_TYPE_BLE},*/
{ BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE },
{ BLE_UUID_CURRENT_TIME_SERVICE, BLE_UUID_TYPE_BLE },
{ BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE } }; /**< Universally unique service identifiers. */
#ifdef BLE_DFU_APP_SUPPORT    
static ble_dfu_t m_dfus; /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT    

/* Extern variables ----------------------------------------------------------*/
extern bool BluetoothEnabled;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*************************  INITIALIZERS  *************************************/
/******************************************************************************/

/*******************************************************************************
 * Function Name  : Init  BLE stack
 *
 * Description    : Initializes the SoftDevice and the BLE event interrupt
 *
 *******************************************************************************/
void ble_stack_init(void)
{
	uint32_t err_code;

	// Initialize the SoftDevice handler module.
	SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

#if defined(S110) || defined(S130) || defined(S310)
	// Enable BLE stack.
	ble_enable_params_t ble_enable_params;
	memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#if defined(S130) || defined(S310)
	ble_enable_params.gatts_enable_params.attr_tab_size = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
#endif
	ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
	err_code = sd_ble_enable(&ble_enable_params);
	APP_ERROR_CHECK(err_code);
#endif

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
	APP_ERROR_CHECK(err_code);

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
	APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Init Device Manager
 *
 * Input			 : erase_bonds  Indicates whether bonding information should be cleared 
 *                  from persistent storage during initialization of the Device Manager
 *
 *******************************************************************************/
void device_manager_init(bool erase_bonds)
{
	uint32_t err_code;
	dm_init_param_t
	init_param =
	{	.clear_persistent_data = erase_bonds};
	dm_application_param_t register_param;

	// Initialize persistent storage module.
	err_code = pstorage_init();
	APP_ERROR_CHECK(err_code);

	err_code = dm_init(&init_param);
	APP_ERROR_CHECK(err_code);

	memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

	register_param.sec_param.bond = SEC_PARAM_BOND;
	register_param.sec_param.mitm = SEC_PARAM_MITM;
	register_param.sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
	register_param.sec_param.oob = SEC_PARAM_OOB;
	register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
	register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
	register_param.evt_handler = device_manager_evt_handler;
	register_param.service_type = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

	err_code = dm_register(&m_app_handle, &register_param);
	APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Init GAP
 *
 * Description	 : This function sets up all the necessary GAP (Generic Access Profile) 
 *                  parameters of the device including the device name, 
 *                  appearance, and the preferred connection parameters
 *
 *******************************************************************************/
void gap_params_init(void)
{
	uint32_t err_code;
	ble_gap_conn_params_t gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

	err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DEVICE_NAME, strlen(DEVICE_NAME));
	APP_ERROR_CHECK(err_code);

	//err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_WATCH);

	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));

	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.slave_latency = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Init Advertising functionality
 *
 *******************************************************************************/
void advertising_init(void)
{
	uint32_t err_code;
	ble_advdata_t advdata;

	// Build advertising data struct to pass into @ref ble_advertising_init.
	memset(&advdata, 0, sizeof(advdata));

	advdata.name_type = BLE_ADVDATA_FULL_NAME;
	advdata.include_appearance = true;
	advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
	advdata.uuids_complete.p_uuids = m_adv_uuids;

	ble_adv_modes_config_t options =
	{ 0 };
	options.ble_adv_fast_enabled = BLE_ADV_FAST_ENABLED;
	options.ble_adv_fast_interval = APP_ADV_INTERVAL;
	options.ble_adv_fast_timeout = APP_ADV_TIMEOUT_IN_SECONDS;
	/*
	 
	 options.ble_adv_whitelist_enabled = BLE_ADV_WHITELIST_ENABLED;
	 options.ble_adv_fast_interval     = APP_ADV_FAST_INTERVAL;
	 options.ble_adv_fast_timeout      = APP_ADV_FAST_TIMEOUT;

	 options.ble_adv_slow_enabled      = BLE_ADV_SLOW_ENABLED;
	 options.ble_adv_slow_interval     = APP_ADV_SLOW_INTERVAL;
	 options.ble_adv_slow_timeout      = APP_ADV_SLOW_TIMEOUT;
	 */

	err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
	APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Init Services, that will be used by the application
 *
 * Description	 : Initialize the Current Time, Battery and Device Information services 
 *
 *******************************************************************************/
void services_init(void)
{
	uint32_t err_code;

// Initialize Heart Rate Service.
	/*
	 ble_hrs_init_t hrs_init;
	 uint8_t body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

	 memset(&hrs_init, 0, sizeof(hrs_init));

	 hrs_init.evt_handler                 = NULL;
	 hrs_init.is_sensor_contact_supported = true;
	 hrs_init.p_body_sensor_location      = &body_sensor_location;

	 // Here the sec level for the Heart Rate Service can be changed/increased.
	 BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.cccd_write_perm);
	 BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.read_perm);
	 BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.write_perm);

	 BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.read_perm);
	 BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_bsl_attr_md.write_perm);

	 err_code = ble_hrs_init(&m_hrs, &hrs_init);
	 APP_ERROR_CHECK(err_code);     */

// Initialize Current Time Service.	
	ble_cts_c_init_t cts_init_obj;
	cts_init_obj.evt_handler = on_cts_c_evt;
	cts_init_obj.error_handler = current_time_error_handler;
	err_code = ble_cts_c_init(&m_cts, &cts_init_obj);

	APP_ERROR_CHECK(err_code);

// Initialize Battery Service.
	ble_bas_init_t bas_init;
	memset(&bas_init, 0, sizeof(bas_init));

	// Here the sec level for the Battery Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

	bas_init.evt_handler = NULL;
	bas_init.support_notification = true;
	bas_init.p_report_ref = NULL;
	bas_init.initial_batt_level = 100;

	err_code = ble_bas_init(&m_bas, &bas_init);
	APP_ERROR_CHECK(err_code);

// Initialize Device Information Service.
	ble_dis_init_t dis_init;
	memset(&dis_init, 0, sizeof(dis_init));

	ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *) MANUFACTURER_NAME);

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);

#ifdef BLE_DFU_APP_SUPPORT
	/** @snippet [DFU BLE Service initialization] */
	ble_dfu_init_t dfus_init;

	// Initialize the Device Firmware Update Service.
	memset(&dfus_init, 0, sizeof(dfus_init));

	dfus_init.evt_handler = dfu_app_on_dfu_evt;
	dfus_init.error_handler = NULL;
	dfus_init.evt_handler = dfu_app_on_dfu_evt;
	dfus_init.revision = DFU_REVISION;

	err_code = ble_dfu_init(&m_dfus, &dfus_init);
	APP_ERROR_CHECK(err_code);

	dfu_app_reset_prepare_set(reset_prepare);
	dfu_app_dm_appl_instance_set(m_app_handle);
	/** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT
}

/*******************************************************************************
 * Function Name  : Init Connection Parameters module
 *
 *******************************************************************************/
void conn_params_init(void)
{
	uint32_t err_code;
	ble_conn_params_init_t cp_init;

	memset(&cp_init, 0, sizeof(cp_init));

	cp_init.p_conn_params = NULL;
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
	cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
	cp_init.start_on_notify_cccd_handle = m_hrs.hrm_handles.cccd_handle;
	cp_init.disconnect_on_fail = false;
	cp_init.evt_handler = on_conn_params_evt;
	cp_init.error_handler = conn_params_error_handler;

	err_code = ble_conn_params_init(&cp_init);
	APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Init Database discovery collector
 *
 *******************************************************************************/
void db_discovery_init(void)
{
	uint32_t err_code = ble_db_discovery_init();
	APP_ERROR_CHECK(err_code);
}

/******************************************************************************/
/*************************  DISPATCHERS  **************************************/
/******************************************************************************/

/*******************************************************************************
 * Function Name  : Dispatching a BLE stack event to all modules with a BLE stack event handler
 *
 * Description    : This function is called from the BLE Stack event interrupt handler 
 *          		   after a BLE stack event has been received
 *
 * Input			 : p_ble_evt  Bluetooth stack event
 *
 *******************************************************************************/
void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
	on_ble_evt(p_ble_evt);
	ble_advertising_on_ble_evt(p_ble_evt);
	ble_conn_params_on_ble_evt(p_ble_evt);
	dm_ble_evt_handler(p_ble_evt);
	bsp_btn_ble_on_ble_evt(p_ble_evt);

	ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);

	ble_bas_on_ble_evt(&m_bas, p_ble_evt);
	ble_cts_c_on_ble_evt(&m_cts, p_ble_evt);
	//ble_hrs_on_ble_evt(&m_hrs, p_ble_evt);

#ifdef BLE_DFU_APP_SUPPORT
	/* Propagating BLE Stack events to DFU Service */
	ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
#endif // BLE_DFU_APP_SUPPORT
}

/*******************************************************************************
 * Function Name  : Dispatching a system event to interested modules
 *
 * Description    : This function is called from the System event interrupt handler after a system
 *          		   event has been received
 *
 * Input			 : sys_evt  System stack event
 *
 *******************************************************************************/
void sys_evt_dispatch(uint32_t sys_evt)
{
	pstorage_sys_event_handler(sys_evt);
	ble_advertising_on_sys_evt(sys_evt);
}

/******************************************************************************/
/*************************  EVENTS  *******************************************/
/******************************************************************************/

/*******************************************************************************
 * Function Name  : Handling the Application's BLE Stack events
 *
 * Input			 : p_ble_evt  Bluetooth stack event
 *
 *******************************************************************************/
void on_ble_evt(ble_evt_t * p_ble_evt)
{
	uint32_t err_code;

	switch (p_ble_evt->header.evt_id)
	{
	case BLE_GAP_EVT_CONNECTED:
		err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
		APP_ERROR_CHECK(err_code);
		m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
		break;

	case BLE_GAP_EVT_DISCONNECTED:
		m_conn_handle = BLE_CONN_HANDLE_INVALID;
		break;

	default:
		// No implementation needed.
		break;
	}
}

/*******************************************************************************
 * Function Name  : Handling the Connection Parameters Module
 *
 * Description	 : This function will be called for all events in the 
 *          		   Connection Parameters Module which are passed to the application.
 *
 *          			NOTE: All this function does is to disconnect. This could have been done by simply
 *                	setting the disconnect_on_fail config parameter, but instead we use the event
 *                	handler mechanism to demonstrate its use.
 *
 * Input			 : p_evt  Event received from the Connection Parameters Module
 *
 *******************************************************************************/
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
	if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
	{
		uint32_t err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
		APP_ERROR_CHECK(err_code);
	}
}

/*******************************************************************************
 * Function Name  : Handling advertising events
 *
 * Description	 : This function will be called for advertising events which are passed to the application 
 *
 * Input			 : ble_adv_evt  Advertising event
 *
 *******************************************************************************/
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
	uint32_t err_code;

	switch (ble_adv_evt)
	{
	case BLE_ADV_EVT_FAST:
		err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
		APP_ERROR_CHECK(err_code);
		break;
	case BLE_ADV_EVT_IDLE:
		LogAddInfo("BT OffA", LOG_WARN, 0);
		nrf_gpio_pin_set (LED_WHT);
		//sleep_mode_enter();
		BluetoothEnabled = false;
		break;
	default:
		break;
	}
}

/*******************************************************************************
 * Function Name  : Handling the Current Time Service client events
 *
 * Description	 : This function will be called for all events in the Current Time Service client that
 *                  are passed to the application 
 *
 * Input			 : p_evt Event received from the Current Time Service client
 *
 *******************************************************************************/
void on_cts_c_evt(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt)
{
	switch (p_evt->evt_type)
	{
	case BLE_CTS_C_EVT_DISCOVERY_COMPLETE:
		//printf("Current Time Service discovered on server.\n");
		break;

	case BLE_CTS_C_EVT_SERVICE_NOT_FOUND:
		//printf("Current Time Service not found on server.\n");
		break;

	case BLE_CTS_C_EVT_DISCONN_COMPLETE:
		//printf("Disconnect Complete.\n");
		break;

	case BLE_CTS_C_EVT_CURRENT_TIME:
		//printf("Current Time received.\n");
		//current_time_print(p_evt);
		break;

	case BLE_CTS_C_EVT_INVALID_TIME:
		//printf("Invalid Time received.\n");
		break;

	default:
		break;
	}
}

/******************************************************************************/
/*************************  HANDLERS  *****************************************/
/******************************************************************************/

/*******************************************************************************
 * Function Name  : Handling the Device Manager events
 *
 * Description	 : p_evt  Data associated to the device manager event 
 *
 *******************************************************************************/
uint32_t device_manager_evt_handler(dm_handle_t const * p_handle, dm_event_t const * p_event, ret_code_t event_result)
{
	uint32_t err_code;

	APP_ERROR_CHECK(event_result);

	switch (p_event->event_id)
	{
	case DM_EVT_CONNECTION:
		m_peer_handle = (*p_handle);
		/*!!! err_code      = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
		 APP_ERROR_CHECK(err_code);*/
		break;

	case DM_EVT_LINK_SECURED:
		err_code = ble_db_discovery_start(&m_ble_db_discovery, p_event->event_param.p_gap_param->conn_handle);
		APP_ERROR_CHECK(err_code);
		break;

	default:
		// No implementation needed.
		break;

	}

#ifdef BLE_DFU_APP_SUPPORT
	if (p_event->event_id == DM_EVT_LINK_SECURED)
	{
		app_context_load(p_handle);
	}
#endif // BLE_DFU_APP_SUPPORT
	return NRF_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Handling a Connection Parameters error
 *
 * Input     	 : nrf_error  Error code containing information about what went wrong 
 *
 *******************************************************************************/
void conn_params_error_handler(uint32_t nrf_error)
{
	APP_ERROR_HANDLER(nrf_error);
}

/*******************************************************************************
 * Function Name  : Handling the Current Time Service errors
 *
 * Description	 : nrf_error  Error code containing information about what went wrong 
 *
 *******************************************************************************/
void current_time_error_handler(uint32_t nrf_error)
{
	nrf_gpio_pin_set (LED_BLU);
	nrf_delay_ms(500);
	APP_ERROR_HANDLER(nrf_error);
}

/******************************************************************************/
/******************  SERVICE  FUNCTIONS  **************************************/
/******************************************************************************/
void BLE_Stop()
{
	// If connected, disconnect
	if(m_conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		uint32_t err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
	}
	advertising_stop();
}

void advertising_start(void)
{
	uint32_t err_code;

	err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
	APP_ERROR_CHECK(err_code);
	LogAddInfo("BT On", LOG_INFO, 0);
	BluetoothEnabled = true;
}

void advertising_stop(void)
{
	uint32_t err_code;

	err_code = sd_ble_gap_adv_stop();
	APP_ERROR_CHECK(err_code);
	LogAddInfo("BT Off", LOG_INFO, 0);

	//err_code = bsp_indication_set(BSP_INDICATE_IDLE);
	//APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Function for putting the chip into sleep mode
 *
 * Description	 : NOTE: This function will not return 
 *
 *******************************************************************************/
void sleep_mode_enter(void)
{
	nrf_gpio_pin_set (LED_WHT);

	uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
	APP_ERROR_CHECK(err_code);

	// Go to system-off mode (this function will not return; wakeup will cause a reset).
	err_code = sd_power_system_off();
	APP_ERROR_CHECK(err_code);
}

/******************************************************************************/
/*************************  DFU  **********************************************/
/******************************************************************************/

#ifdef BLE_DFU_APP_SUPPORT
/*******************************************************************************
 * Function Name  : Function for stopping advertising
 *
 *******************************************************************************/
void advertising_stop(void)
{
	uint32_t err_code;

	err_code = sd_ble_gap_adv_stop();
	APP_ERROR_CHECK(err_code);

	//err_code = bsp_indication_set(BSP_INDICATE_IDLE);
	//APP_ERROR_CHECK(err_code);
}

/*******************************************************************************
 * Function Name  : Function for loading application-specific context after establishing a secure connection
 *
 * Description	 : This function will load the application context and check if the ATT table is marked as 
 *				   changed. If the ATT table is marked as changed, a Service Changed Indication
 *				   is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * Input			 : p_handle The Device Manager handle that identifies the connection for which the context 
 *                  should be loaded
 *
 *******************************************************************************/
void app_context_load(dm_handle_t const * p_handle)
{
	uint32_t err_code;
	static uint32_t context_data;
	dm_application_context_t context;

	context.len = sizeof(context_data);
	context.p_data = (uint8_t *)&context_data;

	err_code = dm_application_context_get(p_handle, &context);
	if (err_code == NRF_SUCCESS)
	{
		// Send Service Changed Indication if ATT table has changed.
		if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
		{
			err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
			if ((err_code != NRF_SUCCESS) &&
				(err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
				(err_code != NRF_ERROR_INVALID_STATE) &&
				(err_code != BLE_ERROR_NO_TX_BUFFERS) &&
				(err_code != NRF_ERROR_BUSY) &&
				(err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
			{
				APP_ERROR_HANDLER(err_code);
			}
		}

		err_code = dm_application_context_delete(p_handle);
		APP_ERROR_CHECK(err_code);
	}
	else if (err_code == DM_NO_APP_CONTEXT)
	{
		// No context available. Ignore.
	}
	else
	{
		APP_ERROR_HANDLER(err_code);
	}
}

/*******************************************************************************
 * Function Name  : DFU BLE Reset prepare (Function for preparing for system reset)
 *
 * Description	 : This function implements @ref dfu_app_reset_prepare_t. It will be called by 
 *                  dfu_app_handler.c before entering the bootloader/DFU.
 *                  This allows the current running application to shut down gracefully
 *
 *******************************************************************************/
void reset_prepare(void)
{
	uint32_t err_code;

	if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		// Disconnect from peer.
		err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		err_code = bsp_indication_set(BSP_INDICATE_IDLE);
		APP_ERROR_CHECK(err_code);
	}
	else
	{
		// If not connected, the device will be advertising. Hence stop the advertising.
		advertising_stop();
	}

	err_code = ble_conn_params_stop();
	APP_ERROR_CHECK(err_code);

	nrf_delay_ms(500);
}
#endif // BLE_DFU_APP_SUPPORT
