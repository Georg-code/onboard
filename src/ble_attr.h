/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BT_ALAMR_H_
#define BT_ALAMR_H_

/**@file
 * @defgroup bt_lbs LED Button Service API
 * @{
 * @brief API for the LED Button Service (ALAMR).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

/** @brief ALAMR Service UUID. */
#define BT_UUID_ALAMR_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd124)


/** @brief LED Characteristic UUID. */
#define BT_UUID_ALAMR_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd124)


#define BT_UUID_ALAMR BT_UUID_DECLARE_128(BT_UUID_ALAMR_VAL)
#define BT_UUID_ALAMR_LED BT_UUID_DECLARE_128(BT_UUID_ALAMR_LED_VAL)
/* STEP 11.2 - Convert the array to a generic UUID */
/** @brief Callback type for when an LED state change is received. */
typedef void (*led_cb_t)(const bool led_state);

/** @brief Callback type for when the button state is pulled. */
typedef bool (*button_cb_t)(void);

/** @brief Callback struct used by the ALAMR Service. */
struct onboard {
	/** LED state change callback. */
	led_cb_t led_cb;
	/** Button read callback. */
	button_cb_t button_cb;
};

/** @brief Initialize the ALAMR Service.
 *
 * This function registers application callback functions with the My ALAMR
 * Service
 *
 * @param[in] callbacks Struct containing pointers to callback functions
 *			used by the service. This pointer can be NULL
 *			if no callback functions are defined.
 *
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int onboard_init(struct onboard *callbacks);

/** @brief Send the button state as indication.
 *
 * This function sends a binary state, typically the state of a
 * button, to all connected peers.
 *
 * @param[in] button_state The state of the button.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int onboard_send_button_state_indicate(bool button_state);

/** @brief Send the button state as notification.
 *
 * This function sends a binary state, typically the state of a
 * button, to all connected peers.
 *
 * @param[in] button_state The state of the button.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int onboard_send_button_state_notify(bool button_state);

/** @brief Send the sensor value as notification.
 *
 * This function sends an uint32_t  value, typically the value
 * of a simulated sensor to all connected peers.
 *
 * @param[in] sensor_value The value of the simulated sensor.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int onboard_send_sensor_notify(uint32_t sensor_value);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* BT_ALAMR_H_ */
