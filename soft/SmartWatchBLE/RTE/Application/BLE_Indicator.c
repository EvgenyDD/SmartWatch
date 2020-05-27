
#include "ble_app.h"
#include "app_timer.h"

uint32_t bsp_indication_set(bsp_indication_t indicate)
{
	uint32_t err_code = NRF_SUCCESS;

#if LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)

	if (m_indication_type & BSP_INIT_LED)
	{
		err_code = bsp_led_indication(indicate);
	}

#endif // LEDS_NUMBER > 0 && !(defined BSP_SIMPLE)
	return err_code;
}

uint32_t bsp_led_indication(bsp_indication_t indicate)
{
	uint32_t err_code = NRF_SUCCESS;
	uint32_t next_delay = 0;

	switch (indicate)
	{
	case BSP_INDICATE_IDLE:
		//LEDS_OFF(LEDS_MASK & ~ALERT_LED_MASK);
		//m_stable_state = indicate;
		break;

	case BSP_INDICATE_SCANNING:
	case BSP_INDICATE_ADVERTISING:
		//LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK & ~ALERT_LED_MASK);

		// in advertising blink LED_0
		/*if (LED_IS_ON(BSP_LED_0_MASK))
		 {
		 //LEDS_OFF(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING ? ADVERTISING_LED_OFF_INTERVAL :
		 ADVERTISING_SLOW_LED_OFF_INTERVAL;
		 }
		 else
		 {
		 //LEDS_ON(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING ? ADVERTISING_LED_ON_INTERVAL :
		 ADVERTISING_SLOW_LED_ON_INTERVAL;
		 }
		 */
		//m_stable_state = indicate;
		//err_code       = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(next_delay), NULL);
		break;

	case BSP_INDICATE_ADVERTISING_WHITELIST:
		//LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK & ~ALERT_LED_MASK);

		// in advertising quickly blink LED_0
		/* if (LED_IS_ON(BSP_LED_0_MASK))
		 {
		 LEDS_OFF(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING_WHITELIST ?
		 ADVERTISING_WHITELIST_LED_OFF_INTERVAL :
		 ADVERTISING_SLOW_LED_OFF_INTERVAL;
		 }
		 else
		 {
		 LEDS_ON(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING_WHITELIST ?
		 ADVERTISING_WHITELIST_LED_ON_INTERVAL :
		 ADVERTISING_SLOW_LED_ON_INTERVAL;
		 }        */
		//m_stable_state = indicate;
		//err_code       = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(next_delay), NULL);
		break;

	case BSP_INDICATE_ADVERTISING_SLOW:
		/*LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK & ~ALERT_LED_MASK);

		 // in advertising slowly blink LED_0
		 if (LED_IS_ON(BSP_LED_0_MASK))
		 {
		 LEDS_OFF(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_OFF_INTERVAL :
		 ADVERTISING_SLOW_LED_OFF_INTERVAL;
		 }
		 else
		 {
		 LEDS_ON(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_ON_INTERVAL :
		 ADVERTISING_SLOW_LED_ON_INTERVAL;
		 }              */
		//m_stable_state = indicate;
		//err_code       = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(next_delay), NULL);
		break;

	case BSP_INDICATE_ADVERTISING_DIRECTED:
		/*LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK & ~ALERT_LED_MASK);

		 // in advertising very quickly blink LED_0
		 if (LED_IS_ON(BSP_LED_0_MASK))
		 {
		 LEDS_OFF(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING_DIRECTED ?
		 ADVERTISING_DIRECTED_LED_OFF_INTERVAL :
		 ADVERTISING_SLOW_LED_OFF_INTERVAL;
		 }
		 else
		 {
		 LEDS_ON(BSP_LED_0_MASK);
		 next_delay = indicate ==
		 BSP_INDICATE_ADVERTISING_DIRECTED ?
		 ADVERTISING_DIRECTED_LED_ON_INTERVAL :
		 ADVERTISING_SLOW_LED_ON_INTERVAL;
		 }     */
		//m_stable_state = indicate;
		//err_code       = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(next_delay), NULL);
		break;

	case BSP_INDICATE_BONDING:
		/*LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK & ~ALERT_LED_MASK);

		 // in bonding fast blink LED_0
		 if (LED_IS_ON(BSP_LED_0_MASK))
		 {
		 LEDS_OFF(BSP_LED_0_MASK);
		 }
		 else
		 {
		 LEDS_ON(BSP_LED_0_MASK);
		 }
		 */
		//m_stable_state = indicate;
		//err_code       =                  app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(BONDING_INTERVAL), NULL);
		break;

	case BSP_INDICATE_CONNECTED:
		//LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK & ~ALERT_LED_MASK);
		//LEDS_ON(BSP_LED_0_MASK);
		//m_stable_state = indicate;
		break;

	case BSP_INDICATE_SENT_OK:
		// when sending shortly invert LED_1
		//LEDS_INVERT(BSP_LED_1_MASK);
		//err_code = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(SENT_OK_INTERVAL), NULL);
		break;

	case BSP_INDICATE_SEND_ERROR:
		// on receving error invert LED_1 for long time
		//LEDS_INVERT(BSP_LED_1_MASK);
		//err_code = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(SEND_ERROR_INTERVAL), NULL);
		break;

	case BSP_INDICATE_RCV_OK:
		// when receving shortly invert LED_1
		//LEDS_INVERT(BSP_LED_1_MASK);
		//err_code = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(RCV_OK_INTERVAL), NULL);
		break;

	case BSP_INDICATE_RCV_ERROR:
		// on receving error invert LED_1 for long time
		//LEDS_INVERT(BSP_LED_1_MASK);
		//err_code = app_timer_start(m_leds_timer_id, BSP_MS_TO_TICK(RCV_ERROR_INTERVAL), NULL);
		break;

	case BSP_INDICATE_FATAL_ERROR:
		// on fatal error turn on all leds
		//LEDS_ON(LEDS_MASK);
		break;

	case BSP_INDICATE_ALERT_0:
	case BSP_INDICATE_ALERT_1:
	case BSP_INDICATE_ALERT_2:
	case BSP_INDICATE_ALERT_3:
	case BSP_INDICATE_ALERT_OFF:
		//err_code   = app_timer_stop(m_alert_timer_id);
		next_delay = (uint32_t) BSP_INDICATE_ALERT_OFF - (uint32_t) indicate;

		// a little trick to find out that if it did not fall through ALERT_OFF
		if(next_delay && (err_code == NRF_SUCCESS))
		{
			if(next_delay > 1)
			{
				//err_code = app_timer_start(m_alert_timer_id, BSP_MS_TO_TICK(   (next_delay * ALERT_INTERVAL)), NULL);
			}
			//LEDS_ON(ALERT_LED_MASK);
		}
		else
		{
			//LEDS_OFF(ALERT_LED_MASK);
		}
		break;

	case BSP_INDICATE_USER_STATE_OFF:
		//LEDS_OFF(LEDS_MASK);
		//m_stable_state = indicate;
		break;

	case BSP_INDICATE_USER_STATE_0:
		//LEDS_OFF(LEDS_MASK & ~BSP_LED_0_MASK);
		//LEDS_ON(BSP_LED_0_MASK);
		//m_stable_state = indicate;
		break;

	case BSP_INDICATE_USER_STATE_1:
		//LEDS_OFF(LEDS_MASK & ~BSP_LED_1_MASK);
		//LEDS_ON(BSP_LED_1_MASK);
		//m_stable_state = indicate;
		break;

	case BSP_INDICATE_USER_STATE_2:
		//LEDS_OFF(LEDS_MASK & ~(BSP_LED_0_MASK | BSP_LED_1_MASK));
		//LEDS_ON(BSP_LED_0_MASK | BSP_LED_1_MASK);
		//m_stable_state = indicate;
		break;

	case BSP_INDICATE_USER_STATE_3:

	case BSP_INDICATE_USER_STATE_ON:
		//LEDS_ON(LEDS_MASK);
		//m_stable_state = indicate;
		break;

	default:
		break;
	}

	return err_code;
}
