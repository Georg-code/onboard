
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/device.h>
#include <zephyr/types.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "ble_attr.h"
#include "alarm.h"

/* Define external devices*/

// LED
#define LED0_NODE DT_ALIAS(stled)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Motor
#define MOT0_NODE DT_ALIAS(moled)
static const struct gpio_dt_spec motor = GPIO_DT_SPEC_GET(MOT0_NODE, gpios);

// Buzzer
#define BUZ0_NODE DT_ALIAS(bzled)
static const struct gpio_dt_spec buzzer = GPIO_DT_SPEC_GET(BUZ0_NODE, gpios);

// Button
#define BTN0_NODE DT_ALIAS(albtn)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);

bool btnval;

static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY),
	800,   // Advertising Interval min in ms
	801,   // Advertising Interval max in ms
	NULL); // undirected

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define STACKSIZE 1024
#define PRIORITY 7

// Data that should be streamed over BLE
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_ALAMR_VAL),
};

static void app_led_cb(bool led_state)
{

	enable_alarm(&led, &motor, &buzzer);
}

// define BLE callbacks
static struct onboard app_callbacks = {
	.led_cb = app_led_cb,
};

// Conncection Feedback
static void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err)
	{
		printk("Connection failed (err %u)\n", err);
		return;
	}

	printk("Connected\n");
}

// Disconnection Feedback (might add a warning sound + Vibration)
static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);
	disconnection_warning(&led, &motor, &buzzer);
}

// Connection Callbacks
struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

int main(void)
{
	int ret;

	if (!device_is_ready(led.port))
	{
		return -1;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return -1;
	}

	ret = gpio_pin_configure_dt(&motor, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return -1;
	}

	ret = gpio_pin_configure_dt(&buzzer, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return -1;
	}

	if (!device_is_ready(button.port))
	{
		return -1;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0)
	{
		return -1;
	}

	// configure the Bluetooth
	int err;
	err = bt_enable(NULL);
	if (err)
	{
		return -1;
	}
	bt_conn_cb_register(&connection_callbacks);

	err = onboard_init(&app_callbacks);

	if (err)
	{
		return -1;
	}

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err)
	{
		return -1;
	}
}
