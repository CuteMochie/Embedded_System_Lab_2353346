
/*
 * Exercise 5: Traffic Light Controller
 * Board: BKIT ARM4 - STM32F407
 *
 * Thread 1: LED3 blinks ON/OFF every 1 second.
 * Thread 2: Traffic light cycles:
 *   RED   - 7 seconds
 *   GREEN - 5 seconds
 *   AMBER - 2 seconds
 *
 * Both threads run independently.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* Devicetree aliases */
#define LED3_NODE DT_ALIAS(led0)
#define Y0_NODE    DT_ALIAS(led1)
#define Y1_NODE    DT_ALIAS(led2)

/* GPIO specifications */
static const struct gpio_dt_spec led3 =
    GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec y0 =
    GPIO_DT_SPEC_GET(Y0_NODE, gpios);

static const struct gpio_dt_spec y1 =
    GPIO_DT_SPEC_GET(Y1_NODE, gpios);

/* Thread configuration */
#define STACK_SIZE 1024
#define THREAD_PRIORITY 5

/* Traffic light phase structure */
struct phase {
    const char *name;
    int y0;
    int y1;
    int duration_ms;
};

/* Constant array of traffic light phases */
static const struct phase phases[] = {
    { "RED",   1, 0, 7000 },
    { "GREEN", 0, 1, 5000 },
    { "AMBER", 1, 1, 2000 }
};

/* Thread 1: System alive LED */
void thread_led_blinky(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if (!gpio_is_ready_dt(&led3)) {
        printk("LED3 GPIO not ready!\n");
        return;
    }

    if (gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE) < 0) {
        printk("LED3 configuration failed!\n");
        return;
    }

    while (1) {
        gpio_pin_set_dt(&led3, 1);
        k_msleep(1000);

        gpio_pin_set_dt(&led3, 0);
        k_msleep(1000);
    }
}

/* Thread 2: Traffic light controller */
void thread_traffic_light(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if (!gpio_is_ready_dt(&y0) ||
        !gpio_is_ready_dt(&y1)) {
        printk("Y0/Y1 GPIO not ready!\n");
        return;
    }

    if (gpio_pin_configure_dt(&y0, GPIO_OUTPUT_INACTIVE) < 0 ||
        gpio_pin_configure_dt(&y1, GPIO_OUTPUT_INACTIVE) < 0) {
        printk("Y0/Y1 configuration failed!\n");
        return;
    }

    while (1) {
        for (int i = 0; i < ARRAY_SIZE(phases); i++) {
            /* Apply the current traffic light phase */
            if (gpio_pin_set_dt(&y0, phases[i].y0) < 0 ||
                gpio_pin_set_dt(&y1, phases[i].y1) < 0) {
                printk("[LIGHT] GPIO update failed!\n");
                return;
            }

            /* Print phase name and duration */
            printk("[LIGHT] %s - %d s\n",
                   phases[i].name,
                   phases[i].duration_ms / 1000);

            /* Keep the current phase */
            k_msleep(phases[i].duration_ms);
        }
    }
}

/* Create LED3 blinking thread */
K_THREAD_DEFINE(
    led_thread_id,
    STACK_SIZE,
    thread_led_blinky,
    NULL, NULL, NULL,
    THREAD_PRIORITY,
    0,
    0
);

/* Create traffic light thread */
K_THREAD_DEFINE(
    traffic_thread_id,
    STACK_SIZE,
    thread_traffic_light,
    NULL, NULL, NULL,
    THREAD_PRIORITY,
    0,
    0
);

int main(void)
{
    return 0;
}
