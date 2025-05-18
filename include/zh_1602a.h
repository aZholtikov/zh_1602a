#pragma once

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct // Structure for initial initialization of LCD 1602A.
    {
        uint8_t rs_gpio_number; // GPIO connected to RS of LCD 1602A.
        uint8_t e_gpio_number;  // GPIO connected to E of LCD 1602A.
        uint8_t d0_gpio_number; // GPIO connected to D0 of LCD 1602A. @note Required for 8 bit work mode only.
        uint8_t d1_gpio_number; // GPIO connected to D1 of LCD 1602A. @note Required for 8 bit work mode only.
        uint8_t d2_gpio_number; // GPIO connected to D2 of LCD 1602A. @note Required for 8 bit work mode only.
        uint8_t d3_gpio_number; // GPIO connected to D3 of LCD 1602A. @note Required for 8 bit work mode only.
        uint8_t d4_gpio_number; // GPIO connected to D4 of LCD 1602A.
        uint8_t d5_gpio_number; // GPIO connected to D5 of LCD 1602A.
        uint8_t d6_gpio_number; // GPIO connected to D6 of LCD 1602A.
        uint8_t d7_gpio_number; // GPIO connected to D7 of LCD 1602A.
    } zh_1602a_init_config_t;

    /**
     * @brief Initializes the LCD 1602A.
     *
     * @param[in] config Pointer to 1602A initialized configuration structure. Can point to a temporary variable.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_init(const zh_1602a_init_config_t *config);

    /**
     * @brief Clears the LCD screen.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_lcd_clear(void);

    /**
     * @brief Sets the cursor to a specific position on the LCD.
     *
     * @param[in] row The row number (0 or 1).
     * @param[in] col The column number (0–15).
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_set_cursor(uint8_t row, uint8_t col);

    /**
     * @brief Prints a string to the LCD.
     *
     * @param[in] str Pointer to the string to be displayed.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_print_char(const char *str);

    /**
     * @brief Prints an integer to the LCD.
     *
     * @param[in] num The integer to be displayed.
     *
     * @return ESP_OK if success or an error code otherwise..
     */
    esp_err_t zh_1602a_print_int(int num);

    /**
     * @brief Prints a floating-point number to the LCD.
     *
     * @param[in] num The floating-point number to be displayed.
     * @param[in] precision The number of decimal places to display.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_print_float(float num, uint8_t precision);

    /**
     * @brief Displays a progress bar on a specific row of the LCD.
     *
     * @param[in] row The row number (0 or 1).
     * @param[in] progress The progress percentage (0–100).
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_print_progress_bar(uint8_t row, uint8_t progress);

    /**
     * @brief Clears a specific row on the LCD.
     *
     * @param[in] row The row number (0 or 1).
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_1602a_clear_row(uint8_t row);

#ifdef __cplusplus
}
#endif