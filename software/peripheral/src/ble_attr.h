#ifndef BT_ALAMR_H_
#define BT_ALAMR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

/** @brief ALARM Service UUID. */
#define BT_UUID_ALAMR_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd124)


/** @brief LED Characteristic UUID. */
#define BT_UUID_ALAMR_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd124)


#define BT_UUID_ALAMR BT_UUID_DECLARE_128(BT_UUID_ALAMR_VAL)
#define BT_UUID_ALAMR_LED BT_UUID_DECLARE_128(BT_UUID_ALAMR_LED_VAL)


typedef void (*led_cb_t)(const bool led_state);


typedef bool (*button_cb_t)(void);


struct onboard {
	
	led_cb_t led_cb;

};


int onboard_init(struct onboard *callbacks);

int onboard_send_button_state_indicate(bool button_state);

int onboard_send_button_state_notify(bool button_state);

int onboard_send_sensor_notify(uint32_t sensor_value);

#ifdef __cplusplus
}
#endif


#endif 