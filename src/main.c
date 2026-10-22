#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

/* Khai báo các chân LED dựa trên alias đã cấu hình sẵn trong overlay[cite: 4] */
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec y0   = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec y1   = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

/* Cấu trúc lưu trữ các pha của đèn giao thông[cite: 7] */
struct phase {
    const char *name;
    int y0;
    int y1;
    int ms;
};

/* Mảng hằng số định nghĩa chu kỳ: RED (7s) -> GREEN (5s) -> AMBER (2s)[cite: 7] */
const struct phase traffic_phases[] = {
    { "RED",   1, 0, 7000 },
    { "GREEN", 0, 1, 5000 },
    { "AMBER", 1, 1, 2000 }
};

/* Thread 1: Chớp tắt LED3 báo hệ thống hoạt động (1s ON, 1s OFF)[cite: 7] */
void thread_led_blinky(void *p1, void *p2, void *p3) {
    if (!gpio_is_ready_dt(&led3)) {
        return;
    }
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);

    while (1) {
        gpio_pin_set_dt(&led3, 1);
        k_msleep(1000);
        gpio_pin_set_dt(&led3, 0);
        k_msleep(1000);
    }
}

/* Thread 2: Điều khiển đèn giao thông Y0 và Y1 bằng mảng trạng thái[cite: 7] */
void thread_traffic_light(void *p1, void *p2, void *p3) {
    if (!gpio_is_ready_dt(&y0) || !gpio_is_ready_dt(&y1)) {
        return;
    }
    gpio_pin_configure_dt(&y0, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&y1, GPIO_OUTPUT_ACTIVE);

    int num_phases = sizeof(traffic_phases) / sizeof(traffic_phases[0]);

    while (1) {
        for (int i = 0; i < num_phases; i++) {
            /* Cập nhật trạng thái đèn Y0 và Y1 */
            gpio_pin_set_dt(&y0, traffic_phases[i].y0);
            gpio_pin_set_dt(&y1, traffic_phases[i].y1);
            
            /* In log ra console trên mỗi lần chuyển pha[cite: 7] */
            printf("[LIGHT] %s - %d s\n", traffic_phases[i].name, traffic_phases[i].ms / 1000);
            
            /* Chờ theo thời gian của pha hiện tại */
            k_msleep(traffic_phases[i].ms);
        }
    }
}

/* Tạo 2 luồng độc lập với kích thước stack 1024 và mức ưu tiên 7[cite: 5, 7] */
K_THREAD_DEFINE(blinky_tid,  1024, thread_led_blinky,    NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(traffic_tid, 1024, thread_traffic_light, NULL, NULL, NULL, 7, 0, 0);

int main(void) {
    /* Hàm main để trống vì các luồng đã tự động chạy[cite: 5] */
    return 0;
}
