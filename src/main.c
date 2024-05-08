/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <dk_buttons_and_leds.h>
#include "ble_attr.h"



#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* STEP 7 - Change the sleep time from 1000 ms to 100 ms */
#define SLEEP_TIME_MS   100

/* STEP 3.1 - Get the node identifier for button 1 through its alias sw0 */
#define SW0_NODE	DT_ALIAS(albtn)

/* STEP 3.2 - Get the device pointer. pin number, and pin's configuration flags through gpio_dt_spec */
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

/* LED0_NODE is the devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(stled)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);






static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL); /* Set to NULL for undirected advertising */

LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)



#define STACKSIZE 1024
#define PRIORITY 7

#define RUN_LED_BLINK_INTERVAL 1000
/* STEP 17 - Define the interval at which you want to send data at */
#define NOTIFY_INTERVAL 500

/* STEP 15 - Define the data you want to stream over Bluetooth LE */
static uint32_t app_sensor_value = 100;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_ALAMR_VAL),
};

/* STEP 16 - Define a function to simulate the data */
static void simulate_data(void)
{
	app_sensor_value++;
	if (app_sensor_value == 200) {
		app_sensor_value = 100;
	}
}
static void app_led_cb(bool led_state)
{

	gpio_pin_set_dt(&led,led_state);
}



/* STEP 18.1 - Define the thread function  */
void send_data_thread(void)
{
	while (1) {
		/* Simulate data */
		simulate_data();
		/* Send notification, the function sends notifications only if a client is subscribed */
		onboard_send_sensor_notify(app_sensor_value);

		k_sleep(K_MSEC(NOTIFY_INTERVAL));
	}
}

static struct onboard app_callbacks = {
	.led_cb = app_led_cb,

};


static void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
		return;
	}

	printk("Connected\n");


}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);


}

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};




int main(void)
{
	int err;

	LOG_INF("Starting Lesson 4 - Exercise 2 \n");




	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}


	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	bt_conn_cb_register(&connection_callbacks);

	err = onboard_init(&app_callbacks);
	if (err) {
		printk("Failed to init OnBoard Systems (err:%d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}

	LOG_INF("Advertising successfully started. You are ready to be safed\n");

	
int ret;

	if (!device_is_ready(led.port)) {
		return -1;
	}
	/* STEP 4 - Verify that the device is ready for use */
	if (!device_is_ready(button.port)) {
		return -1;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}

	/* STEP 5 - Configure the pin connected to the button to be an input pin and set its hardware specifications */
	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return -1;
	}
	while (1) {
		/* STEP 6.1 - Read the status of the button and store it */
        bool val = gpio_pin_get_dt(&button);

		/* STEP 6.2 - Update the LED to the status of the button */
        gpio_pin_set_dt(&led,val);

        k_msleep(SLEEP_TIME_MS); // Put the main thread to sleep for 100ms for power optimization
	}


}
/* STEP 18.2 - Define and initialize a thread to send data periodically */
K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
