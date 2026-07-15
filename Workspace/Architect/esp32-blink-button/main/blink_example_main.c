/* Blink Example with Button Control

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "example";

/* LED GPIO - configured via menuconfig */
#define BLINK_GPIO CONFIG_BLINK_GPIO

/* Button GPIO - 默认GPIO19 */
#define BUTTON_GPIO 19

/* Debounce time in milliseconds */
#define DEBOUNCE_TIME_MS 200

static uint8_t s_led_state = 0;

/* 10种颜色 RGB (亮度约25%) */
#define BRIGHTNESS 64
static const struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} s_colors[] = {
    {BRIGHTNESS, 0,       0      },  /* 红 */
    {0,         BRIGHTNESS, 0    },  /* 绿 */
    {0,         0,         BRIGHTNESS},  /* 蓝 */
    {BRIGHTNESS, BRIGHTNESS, 0    },  /* 黄 */
    {0,         BRIGHTNESS, BRIGHTNESS},  /* 青 */
    {BRIGHTNESS, 0,       BRIGHTNESS},  /* 紫 */
    {BRIGHTNESS, BRIGHTNESS/2, 0},  /* 橙 */
    {BRIGHTNESS/2, BRIGHTNESS, BRIGHTNESS/2},  /* 粉 */
    {0,         BRIGHTNESS/2, BRIGHTNESS},  /* 蓝绿 */
    {BRIGHTNESS, BRIGHTNESS, BRIGHTNESS},  /* 白 */
};
static uint8_t s_color_index = 0;

/* 按钮任务 - 轮询方式 */
static void button_task(void *arg)
{
    uint8_t last_state = 1;
    uint64_t last_time = 0;

    /* 配置按钮GPIO */
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,  /* 轮询模式，不用中断 */
    };
    ESP_ERROR_CHECK(gpio_config(&btn_cfg));
    ESP_LOGI(TAG, "Button configured on GPIO %d (polling mode)", BUTTON_GPIO);

    while (1) {
        uint8_t state = gpio_get_level(BUTTON_GPIO);
        uint64_t now = esp_timer_get_time() / 1000;

        /* 按钮按下（下降沿）且消抖 */
        if (state == 0 && last_state == 1 && (now - last_time) > DEBOUNCE_TIME_MS) {
            s_color_index = (s_color_index + 1) % 10;
            ESP_LOGI(TAG, "Button pressed! Color %d: RGB(%d,%d,%d)",
                     s_color_index,
                     s_colors[s_color_index].r,
                     s_colors[s_color_index].g,
                     s_colors[s_color_index].b);
            last_time = now;
        }
        last_state = state;
        vTaskDelay(10 / portTICK_PERIOD_MS);  /* 10ms 轮询间隔 */
    }
}

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

/* 更新LED颜色 */
static void update_led_color(void)
{
    uint8_t idx = s_color_index % 10;
    if (s_led_state) {
        led_strip_set_pixel(led_strip, 0, s_colors[idx].r, s_colors[idx].g, s_colors[idx].b);
        led_strip_refresh(led_strip);
        ESP_LOGI(TAG, "LED ON - Color %d: RGB(%d,%d,%d)", idx,
                 s_colors[idx].r, s_colors[idx].g, s_colors[idx].b);
    }
}

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        update_led_color();
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    /* 创建按钮轮询任务 */
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
