#include "zh_1602a.h"

static const char *TAG = "zh_1602a";

#define ZH_1602A_LOGI(msg, ...) ESP_LOGI(TAG, msg, ##__VA_ARGS__)
#define ZH_1602A_LOGW(msg, ...) ESP_LOGW(TAG, msg, ##__VA_ARGS__)
#define ZH_1602A_LOGE(msg, ...) ESP_LOGE(TAG, msg, ##__VA_ARGS__)
#define ZH_1602A_LOGE_ERR(msg, err, ...) ESP_LOGE(TAG, "[%s:%d:%s] " msg, __FILE__, __LINE__, esp_err_to_name(err), ##__VA_ARGS__)

#define ZH_1602A_CHECK(cond, err, msg, ...) \
    if (!(cond))                            \
    {                                       \
        ZH_1602A_LOGE_ERR(msg, err);        \
        return err;                         \
    }

#ifdef CONFIG_IDF_TARGET_ESP8266
#define esp_delay_us(x) os_delay_us(x)
#else
#define esp_delay_us(x) esp_rom_delay_us(x)
#endif

static esp_err_t _zh_1602a_gpio_init(const zh_1602a_init_config_t *config);
static void _zh_1602a_lcd_init(void);
static bool _zh_1602a_8bit_gpio_check(uint8_t rs, uint8_t e, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
static bool _zh_1602a_4bit_gpio_check(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
static void _zh_1602a_send_command(uint8_t command);
static void _zh_1602a_send_data(uint8_t data);
static void _zh_1602a_pulse_enable(void);
static void _zh_1602a_send_8bit(uint8_t data);
static void _zh_1602a_send_4bit(uint8_t data);

static uint8_t _rs_pin = 0;
static uint8_t _e_pin = 0;
static uint8_t _gpio_matrix[8] = {0};
static bool _is_initialized = false;
static bool _is_8bit_work_mode = true;

esp_err_t zh_1602a_init(const zh_1602a_init_config_t *config)
{
    ZH_1602A_LOGI("1602A initialization started.");
    ZH_1602A_CHECK(config != NULL, ESP_ERR_INVALID_ARG, "1602A initialization failed. Invalid argument.");
    ZH_1602A_CHECK(_is_initialized == false, ESP_ERR_INVALID_STATE, "1602A initialization failed. 1602A is already initialized.");
    esp_err_t err = _zh_1602a_gpio_init(config);
    ZH_1602A_CHECK(err == ESP_OK, ESP_FAIL, "1602A initialization failed. GPIO initialization failed.");
    _zh_1602a_lcd_init();
    _is_initialized = true;
    ZH_1602A_LOGI("1602A initialization completed successfully in %d bit mode.", (_is_8bit_work_mode == true) ? 8 : 4);
    return ESP_OK;
}

esp_err_t zh_1602a_lcd_clear(void)
{
    ZH_1602A_LOGI("1602A display cleaning started.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A display cleaning failed. 1602A not initialized.");
    _zh_1602a_send_command(0x01);
    esp_delay_us(1600);
    ZH_1602A_LOGI("1602A display cleaning completed successfully.");
    return ESP_OK;
}

esp_err_t zh_1602a_set_cursor(uint8_t row, uint8_t col)
{
    ZH_1602A_LOGI("1602A set cursor started.");
    ZH_1602A_CHECK(row < 2 && col < 16, ESP_ERR_INVALID_ARG, "1602A set cursor failed. Invalid argument.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A set cursor failed. 1602A not initialized.");
    uint8_t address = (row == 0) ? col : (0x40 + col);
    _zh_1602a_send_command(0x80 | address);
    ZH_1602A_LOGI("1602A set cursor completed successfully.");
    return ESP_OK;
}

esp_err_t zh_1602a_print_char(const char *str)
{
    ZH_1602A_LOGI("1602A print char started.");
    ZH_1602A_CHECK(str != NULL, ESP_ERR_INVALID_ARG, "1602A print char failed. Invalid argument.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A print char failed. 1602A not initialized.");
    while (*str != 0)
    {
        _zh_1602a_send_data((uint8_t)*str++);
    }
    ZH_1602A_LOGI("1602A print char completed successfully.");
    return ESP_OK;
}

esp_err_t zh_1602a_print_int(int num)
{
    ZH_1602A_LOGI("1602A print int started.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A print int failed. 1602A not initialized.");
    char buffer[12];
    sprintf(buffer, "%d", num);
    zh_1602a_print_char(buffer);
    ZH_1602A_LOGI("1602A print int completed successfully.");
    return ESP_OK;
}

esp_err_t zh_1602a_print_float(float num, uint8_t precision)
{
    ZH_1602A_LOGI("1602A print float started.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A print float failed. 1602A not initialized.");
    char buffer[16];
    sprintf(buffer, "%.*f", precision, num);
    zh_1602a_print_char(buffer);
    ZH_1602A_LOGI("1602A print float completed successfully.");
    return ESP_OK;
}

esp_err_t zh_1602a_print_progress_bar(uint8_t row, uint8_t progress)
{
    ZH_1602A_LOGI("1602A print progress bar started.");
    ZH_1602A_CHECK(row < 2 && progress <= 100, ESP_ERR_INVALID_ARG, "1602A print progress bar failed. Invalid argument.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A print progress bar failed. 1602A not initialized.");
    uint8_t blocks = (progress * 16) / 100;
    zh_1602a_set_cursor(row, 0);
    for (uint8_t i = 0; i < 16; ++i)
    {
        if (i < blocks)
        {
            zh_1602a_print_char("\xFF");
        }
        else
        {
            zh_1602a_print_char(" ");
        }
    }
    ZH_1602A_LOGI("1602A print progress bar completed successfully.");
    return ESP_OK;
}

esp_err_t zh_1602a_clear_row(uint8_t row)
{
    ZH_1602A_LOGI("1602A clear row started.");
    ZH_1602A_CHECK(row < 2, ESP_ERR_INVALID_ARG, "1602A clear row failed. Invalid argument.");
    ZH_1602A_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, "1602A clear row failed. 1602A not initialized.");
    zh_1602a_set_cursor(row, 0);
    for (uint8_t i = 0; i < 16; ++i)
    {
        zh_1602a_print_char(" ");
    }
    zh_1602a_set_cursor(row, 0);
    ZH_1602A_LOGI("1602A clear row completed successfully.");
    return ESP_OK;
}

static esp_err_t _zh_1602a_gpio_init(const zh_1602a_init_config_t *config)
{
    ZH_1602A_CHECK(config->rs_gpio_number < GPIO_NUM_MAX || config->e_gpio_number < GPIO_NUM_MAX || config->d0_gpio_number < GPIO_NUM_MAX ||
                       config->d1_gpio_number < GPIO_NUM_MAX || config->d2_gpio_number < GPIO_NUM_MAX || config->d3_gpio_number < GPIO_NUM_MAX ||
                       config->d4_gpio_number < GPIO_NUM_MAX || config->d5_gpio_number < GPIO_NUM_MAX || config->d6_gpio_number < GPIO_NUM_MAX || config->d7_gpio_number < GPIO_NUM_MAX,
                   ESP_ERR_INVALID_ARG, "Invalid GPIO number.");
    bool gpio_check = _zh_1602a_8bit_gpio_check(config->rs_gpio_number, config->e_gpio_number, config->d0_gpio_number, config->d1_gpio_number, config->d2_gpio_number,
                                                config->d3_gpio_number, config->d4_gpio_number, config->d5_gpio_number, config->d6_gpio_number, config->d7_gpio_number);
    if (gpio_check == false)
    {
        gpio_check = _zh_1602a_4bit_gpio_check(config->rs_gpio_number, config->e_gpio_number, config->d4_gpio_number, config->d5_gpio_number, config->d6_gpio_number, config->d7_gpio_number);
        ZH_1602A_CHECK(gpio_check == true, ESP_FAIL, "Invalid GPIO number.");
        _is_8bit_work_mode = false;
    }
    gpio_config_t pin_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pin_bit_mask = (1ULL << config->rs_gpio_number) | (1ULL << config->e_gpio_number) | (1ULL << config->d0_gpio_number) | (1ULL << config->d1_gpio_number) |
                        (1ULL << config->d2_gpio_number) | (1ULL << config->d3_gpio_number) | (1ULL << config->d4_gpio_number) | (1ULL << config->d5_gpio_number) |
                        (1ULL << config->d6_gpio_number) | (1ULL << config->d7_gpio_number),
    };
    esp_err_t err = gpio_config(&pin_config);
    ZH_1602A_CHECK(err == ESP_OK, err, "GPIO initialization failed.")
    _rs_pin = config->rs_gpio_number;
    _e_pin = config->e_gpio_number;
    _gpio_matrix[0] = config->d0_gpio_number;
    _gpio_matrix[1] = config->d1_gpio_number;
    _gpio_matrix[2] = config->d2_gpio_number;
    _gpio_matrix[3] = config->d3_gpio_number;
    _gpio_matrix[4] = config->d4_gpio_number;
    _gpio_matrix[5] = config->d5_gpio_number;
    _gpio_matrix[6] = config->d6_gpio_number;
    _gpio_matrix[7] = config->d7_gpio_number;
    return ESP_OK;
}

static void _zh_1602a_lcd_init(void)
{
    vTaskDelay(20 / portTICK_PERIOD_MS);
    _zh_1602a_send_4bit(0x03);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    _zh_1602a_send_4bit(0x03);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    _zh_1602a_send_4bit(0x03);
    esp_delay_us(150);
    if (_is_8bit_work_mode == false)
    {
        _zh_1602a_send_4bit(0x02);
    }
    _zh_1602a_send_command((_is_8bit_work_mode == true) ? 0x38 : 0x28);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    _zh_1602a_send_command((_is_8bit_work_mode == true) ? 0x38 : 0x28);
    esp_delay_us(150);
    _zh_1602a_send_command((_is_8bit_work_mode == true) ? 0x38 : 0x28);
    _zh_1602a_send_command(0x0C);
    _zh_1602a_send_command(0x01);
    _zh_1602a_send_command(0x06);
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

static bool _zh_1602a_8bit_gpio_check(uint8_t rs, uint8_t e, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
    uint8_t matrix[] = {rs, e, d0, d1, d2, d3, d4, d5, d6, d7};
    for (uint8_t i = 0; i < sizeof(matrix); ++i)
    {
        for (uint8_t j = i + 1; j < sizeof(matrix); ++j)
        {
            if (matrix[i] == matrix[j])
            {
                return false;
            }
        }
    }
    return true;
}

static bool _zh_1602a_4bit_gpio_check(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
    uint8_t matrix[] = {rs, e, d4, d5, d6, d7};
    for (uint8_t i = 0; i < sizeof(matrix); ++i)
    {
        for (uint8_t j = i + 1; j < sizeof(matrix); ++j)
        {
            if (matrix[i] == matrix[j])
            {
                return false;
            }
        }
    }
    return true;
}

static void _zh_1602a_send_command(uint8_t command)
{
    gpio_set_level(_rs_pin, 0);
    gpio_set_level(_e_pin, 0);
    if (_is_8bit_work_mode == true)
    {
        _zh_1602a_send_8bit(command);
    }
    else
    {
        _zh_1602a_send_4bit(command >> 4);
        _zh_1602a_send_4bit(command);
    }
}

static void _zh_1602a_send_data(uint8_t data)
{
    gpio_set_level(_rs_pin, 1);
    gpio_set_level(_e_pin, 0);
    if (_is_8bit_work_mode == true)
    {
        _zh_1602a_send_8bit(data);
    }
    else
    {
        _zh_1602a_send_4bit(data >> 4);
        _zh_1602a_send_4bit(data);
    }
}

static void _zh_1602a_pulse_enable(void)
{
    gpio_set_level(_e_pin, 1);
    esp_delay_us(1);
    gpio_set_level(_e_pin, 0);
    esp_delay_us(40);
}

static void _zh_1602a_send_8bit(uint8_t data)
{
    for (uint8_t i = 0; i <= 7; ++i)
    {
        gpio_set_level(_gpio_matrix[i], (data >> i) & 0x01);
    }
    _zh_1602a_pulse_enable();
}

static void _zh_1602a_send_4bit(uint8_t data)
{
    for (uint8_t i = 0; i <= 3; ++i)
    {
        gpio_set_level(_gpio_matrix[i + 4], (data >> i) & 0x01);
    }
    _zh_1602a_pulse_enable();
}