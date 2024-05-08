#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "ble_attr.h"

static bool indicate_enabled;
static bool button_state;
static struct onboard board;


static struct bt_gatt_indicate_params ind_params;

// config changes
static void onboard_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}


// This function is called when a remote device has acknowledged the indication at its host layer
static void indicate_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{

}
static ssize_t write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{
	
	if (len != 1U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (board.led_cb) {
		uint8_t val = *((uint8_t *)buf);

		if (val == 0x00 || val == 0x01) {
			board.led_cb(val ? true : false);
		} else {
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
	}

	return len;
}



/* LED Button Service Declaration */
BT_GATT_SERVICE_DEFINE(
	onboard_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_ALAMR),
	BT_GATT_CCC(onboard_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

	BT_GATT_CHARACTERISTIC(BT_UUID_ALAMR_LED, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
			       write_led, NULL),
	


);

int onboard_init(struct onboard *callbacks)
{
	if (callbacks) {
		board.led_cb = callbacks->led_cb;
	}

	return 0;
}


