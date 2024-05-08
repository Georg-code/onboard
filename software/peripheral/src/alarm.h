#ifndef ALARM_H
#define ALARM_H

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

void enable_alarm(const struct gpio_dt_spec *led, const struct gpio_dt_spec *motor, const struct gpio_dt_spec *buzzer);
void disable_alarm(const struct gpio_dt_spec *led, const struct gpio_dt_spec *motor, const struct gpio_dt_spec *buzzer);
void disconnection_warning(const struct gpio_dt_spec *led, const struct gpio_dt_spec *motor, const struct gpio_dt_spec *buzzer);

#endif /* ALARM_H */
