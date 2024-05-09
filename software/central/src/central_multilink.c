/**
 * @file central_multilink.c
 *
 * @brief This file contains the implementation of a central device that can connect to multiple peripheral devices using Bluetooth Low Energy (BLE).
 *
 * The central device scans for nearby BLE devices and connects to them if they are in close proximity. It maintains a count of the connected devices and can handle disconnections and reconnections.
 *
 * The central device also provides functionality to set an alarm property on all connected devices.
 *
 * This code is based on the Zephyr RTOS Bluetooth stack and is compatible with Nordic Semiconductor and Intel Corporation platforms.
 *
 * This code as beed adapted by Georg Niggli for the purpose of implementing a MAN OVER BOARD (MOB) system. The foundational code was provided by Zeiphir.
 */

/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include "central_multilink.h"

static void start_scan(void);

static struct bt_conn *conn_connecting;
static uint8_t volatile conn_count;
static bool volatile is_disconnecting;

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
						 struct net_buf_simple *ad)
{
	struct bt_conn_le_create_param create_param = {
		.options = BT_CONN_LE_OPT_NONE,
		.interval = INIT_INTERVAL,
		.window = INIT_WINDOW,
		.interval_coded = 0,
		.window_coded = 0,
		.timeout = 0,
	};
	struct bt_le_conn_param conn_param = {
		.interval_min = CONN_INTERVAL,
		.interval_max = CONN_INTERVAL,
		.latency = CONN_LATENCY,
		.timeout = CONN_TIMEOUT,
	};
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	if (conn_connecting)
	{
		return;
	}

	/* We're only interested in connectable events */
	if (type != BT_GAP_ADV_TYPE_ADV_IND &&
		type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND &&
		type != BT_GAP_ADV_TYPE_EXT_ADV)
	{
		return;
	}

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	/* connect only to devices in close proximity */
	if (rssi < -50)
	{
		return;
	}

	if (bt_le_scan_stop())
	{
		printk("Scanning successfully stopped\n");
		return;
	}

	err = bt_conn_le_create(addr, &create_param, &conn_param,
							&conn_connecting);
	if (err)
	{
		printk("Create conn to %s failed (%d)\n", addr_str, err);
		start_scan();
	}
}

static void start_scan(void)
{
	struct bt_le_scan_param scan_param = {
		.type = BT_HCI_LE_SCAN_PASSIVE,
		.options = BT_LE_SCAN_OPT_NONE,
		.interval = SCAN_INTERVAL,
		.window = SCAN_WINDOW,
	};
	int err;

	err = bt_le_scan_start(&scan_param, device_found);
	if (err)
	{
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

#if defined(CONFIG_BT_GATT_CLIENT)
static void mtu_exchange_cb(struct bt_conn *conn, uint8_t err,
							struct bt_gatt_exchange_params *params)
{
	printk("MTU exchange %u %s (%u)\n", bt_conn_index(conn),
		   err == 0U ? "successful" : "failed", bt_gatt_get_mtu(conn));
}

static struct bt_gatt_exchange_params mtu_exchange_params[CONFIG_BT_MAX_CONN];

static int mtu_exchange(struct bt_conn *conn)
{
	uint8_t conn_index;
	int err;

	conn_index = bt_conn_index(conn);

	printk("MTU (%u): %u\n", conn_index, bt_gatt_get_mtu(conn));

	mtu_exchange_params[conn_index].func = mtu_exchange_cb;

	err = bt_gatt_exchange_mtu(conn, &mtu_exchange_params[conn_index]);
	if (err)
	{
		printk("MTU exchange failed (err %d)", err);
	}
	else
	{
		printk("Exchange pending...");
	}

	return err;
}
#endif /* CONFIG_BT_GATT_CLIENT */

static void connected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (reason)
	{
		printk("Failed to connect to %s (%u)\n", addr, reason);

		bt_conn_unref(conn_connecting);
		conn_connecting = NULL;

		start_scan();
		return;
	}

	conn_connecting = NULL;

	conn_count++;
	if (conn_count < CONFIG_BT_MAX_CONN)
	{
		start_scan();
	}

	printk("Connected (%u): %s\n", conn_count, addr);
}

static void set_alarm_property(void)
{
	struct bt_conn *conn = NULL;
	struct bt_gatt_exchange_params params;
	uint8_t conn_index;

	// Iterate through all connected devices
	for (conn_index = 0; conn_index < CONFIG_BT_MAX_CONN; conn_index++)
	{
		conn = bt_conn_lookup_index(conn_index, BT_CONN_TYPE_LE);
		if (!conn)
		{
			continue;
		}

		// Set alarm property
		params.func = NULL; // We don't need a callback

		uint8_t alarm_data = 1;																	 // Define the "alarm_data" variable with a value of 1
		bt_gatt_write_without_response(conn, BT_UUID_ALARM, &alarm_data, sizeof(alarm_data), 0); // Pass the address of "alarm_data" and the size of "alarm_data" to the function call
		bt_conn_unref(conn);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	// Set alarm property to 1 on all connected devices
	set_alarm_property();

	bt_conn_unref(conn);

	if ((conn_count == 1U) && is_disconnecting)
	{
		is_disconnecting = false;
		start_scan();
	}
	conn_count--;
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("LE conn param req: %s int (0x%04x, 0x%04x) lat %d to %d\n",
		   addr, param->interval_min, param->interval_max, param->latency,
		   param->timeout);

	return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval,
							 uint16_t latency, uint16_t timeout)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("LE conn param updated: %s int 0x%04x lat %d to %d\n",
		   addr, interval, latency, timeout);
}

#if defined(CONFIG_BT_SMP)
static void security_changed(struct bt_conn *conn, bt_security_t level,
							 enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err)
	{
		printk("Security changed: %s level %u\n", addr, level);
	}
	else
	{
		printk("Security failed: %s level %u err %d\n", addr, level,
			   err);
	}
}
#endif

#if defined(CONFIG_BT_USER_PHY_UPDATE)
static void le_phy_updated(struct bt_conn *conn,
						   struct bt_conn_le_phy_info *param)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("LE PHY Updated: %s Tx 0x%x, Rx 0x%x\n", addr, param->tx_phy,
		   param->rx_phy);
}
#endif /* CONFIG_BT_USER_PHY_UPDATE */

#if defined(CONFIG_BT_USER_DATA_LEN_UPDATE)
static void le_data_len_updated(struct bt_conn *conn,
								struct bt_conn_le_data_len_info *info)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Data length updated: %s max tx %u (%u us) max rx %u (%u us)\n",
		   addr, info->tx_max_len, info->tx_max_time, info->rx_max_len,
		   info->rx_max_time);
}
#endif /* CONFIG_BT_USER_DATA_LEN_UPDATE */

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_req = le_param_req,
	.le_param_updated = le_param_updated,

#if defined(CONFIG_BT_SMP)
	.security_changed = security_changed,
#endif

#if defined(CONFIG_BT_USER_PHY_UPDATE)
	.le_phy_updated = le_phy_updated,
#endif /* CONFIG_BT_USER_PHY_UPDATE */

#if defined(CONFIG_BT_USER_DATA_LEN_UPDATE)
	.le_data_len_updated = le_data_len_updated,
#endif /* CONFIG_BT_USER_DATA_LEN_UPDATE */
};

static void disconnect(struct bt_conn *conn, void *data)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnecting %s...\n", addr);
	err = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	if (err)
	{
		printk("Failed disconnection %s.\n", addr);
	}
	printk("success.\n");
}

int init_central(uint8_t iterations)
{
	int err;

	err = bt_enable(NULL);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	bt_conn_cb_register(&conn_callbacks);

	start_scan();

	while (true)
	{
		while (conn_count < CONFIG_BT_MAX_CONN)
		{
			k_sleep(K_MSEC(10));
		}
	}

	return 0;
}