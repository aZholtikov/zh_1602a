# ESP32 ESP-IDF and ESP8266 RTOS SDK component for liquid crystal display module 1602A

## Tested on

1. [ESP8266 RTOS_SDK v3.4](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/index.html#)
2. [ESP32 ESP-IDF v5.4](https://docs.espressif.com/projects/esp-idf/en/release-v5.4/esp32/index.html)

## Note

1. Automatic selection of 8 bit or 4 bit work mode.
2. Display off and cursor display are not supported.

## Using

In an existing project, run the following command to install the component:

```text
cd ../your_project/components
git clone http://git.zh.com.ru/alexey.zholtikov/zh_1602a
```

In the application, add the component:

```c
#include "zh_1602a.h"
```

## Example

8 bit or 4 bit work mode:

```c
#include "zh_1602a.h"

void app_main(void)
{
    esp_log_level_set("zh_1602a", ESP_LOG_NONE); // For ESP8266 first enable "Component config -> Log output -> Enable log set level" via menuconfig.
    zh_1602a_init_config_t init_config = {
        .rs_gpio_number = GPIO_NUM_4,
        .e_gpio_number = GPIO_NUM_2,
        // .d0_gpio_number = GPIO_NUM_21, // Required for 8 bit work mode only.
        // .d1_gpio_number = GPIO_NUM_13, // Required for 8 bit work mode only.
        // .d2_gpio_number = GPIO_NUM_14, // Required for 8 bit work mode only.
        // .d3_gpio_number = GPIO_NUM_15, // Required for 8 bit work mode only.
        .d4_gpio_number = GPIO_NUM_16,
        .d5_gpio_number = GPIO_NUM_17,
        .d6_gpio_number = GPIO_NUM_18,
        .d7_gpio_number = GPIO_NUM_19,
    };
    zh_1602a_init(&init_config);
    for (;;)
    {
        zh_1602a_set_cursor(0, 0);
        zh_1602a_print_char("LCD 1602A");
        zh_1602a_set_cursor(1, 0);
        zh_1602a_print_char("Hello World!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        zh_1602a_clear_row(0);
        zh_1602a_print_char("Progress: ");
        for (uint8_t i = 0; i <= 100; ++i)
        {
            zh_1602a_set_cursor(0, 10);
            zh_1602a_print_int(i);
            zh_1602a_print_char("%");
            zh_1602a_print_progress_bar(1, i);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        zh_1602a_lcd_clear();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```
