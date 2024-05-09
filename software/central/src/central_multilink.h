#ifndef CENTRAL_MULTILINK_H
#define CENTRAL_MULTILINK_H

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

#define SCAN_INTERVAL 0x0640 /* 1000 ms */
#define SCAN_WINDOW 0x0030   /* 30 ms */
#define INIT_INTERVAL 0x0010 /* 10 ms */
#define INIT_WINDOW 0x0010   /* 10 ms */
#define CONN_INTERVAL 0x0320 /* 1000 ms */
#define CONN_LATENCY 0
#define CONN_TIMEOUT MIN(MAX((CONN_INTERVAL * 125 *               \
                              MAX(CONFIG_BT_MAX_CONN, 6) / 1000), \
                             10),                                 \
                         3200)
#define BT_UUID_ALARM_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd124)
#define BT_UUID_ALARM BT_UUID_DECLARE_128(BT_UUID_ALARM_VAL)

static void start_scan(void);

static struct bt_conn *conn_connecting;

#endif /* CENTRAL_MULTILINK_H */