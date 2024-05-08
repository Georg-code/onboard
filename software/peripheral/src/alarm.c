#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
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

static bool alarm_enabled = false;

void enable_alarm(const struct gpio_dt_spec *led, const struct gpio_dt_spec *motor, const struct gpio_dt_spec *buzzer)
{
    alarm_enabled = true;
    printk("Alarm enabled\n");

    gpio_pin_set_dt(led, 1);
    gpio_pin_set_dt(motor, 1);
    gpio_pin_set_dt(buzzer, 1);
}

void disable_alarm(const struct gpio_dt_spec *led, const struct gpio_dt_spec *motor, const struct gpio_dt_spec *buzzer)
{
    alarm_enabled = false;
    printk("Alarm disabled\n");

    gpio_pin_set_dt(led, 0);
    gpio_pin_set_dt(motor, 0);
    gpio_pin_set_dt(buzzer, 0);
}

void disconnection_warning(const struct gpio_dt_spec *led, const struct gpio_dt_spec *motor, const struct gpio_dt_spec *buzzer)
{
    printk("Disco Warning!!\n");

    if (!alarm_enabled)
    {
        for (int i = 0; i < 10; i++)
        {
            // pins high for 0.5s
            gpio_pin_set_dt(led, 1);
            gpio_pin_set_dt(motor, 1);
            gpio_pin_set_dt(buzzer, 1);
            k_sleep(K_MSEC(100));

            // pins low for 0.5s
            gpio_pin_set_dt(led, 0);
            gpio_pin_set_dt(motor, 0);
            gpio_pin_set_dt(buzzer, 0);
            k_sleep(K_MSEC(100));
        }
        printk("Alarm still enabled\n");
    }
}