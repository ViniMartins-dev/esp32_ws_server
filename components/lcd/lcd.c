
#include <stdio.h>

#include "lcd.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "i2cdev.h"
#include "pcf8574.h"
#include "hd44780.h"

#define I2C_PORT       I2C_NUM_0
#define I2C_SDA        21
#define I2C_SCL        22
#define I2C_FREQ       100000

#define LCD_ADDRESS    0x27

static const char *TAG = "LCD";

static i2c_dev_t pcf_dev;

static hd44780_t lcd = {
    .write_cb = NULL,

    .pins = {
        .rs = 0,
        .e  = 2,
        .d4 = 4,
        .d5 = 5,
        .d6 = 6,
        .d7 = 7,
        .bl = 3
    },

    .font = HD44780_FONT_5X8,
    .lines = 2,
    .backlight = true
};

// Callback de comunicação com o PCF8574
static esp_err_t lcd_write_callback(
    const hd44780_t *lcd,
    uint8_t data
)
{
    return pcf8574_port_write(&pcf_dev, data);
}

// Inicialização do componente
esp_err_t lcd_init(void)
{
    ESP_LOGI(TAG, "Inicializando I2C...");

    ESP_ERROR_CHECK(i2cdev_init());

    ESP_ERROR_CHECK(
        pcf8574_init_desc(
            &pcf_dev,
            LCD_ADDRESS,
            I2C_PORT,
            I2C_SDA,
            I2C_SCL
        )
    );

    lcd.write_cb = lcd_write_callback;

    ESP_LOGI(TAG, "Inicializando display...");

    ESP_ERROR_CHECK(hd44780_init(&lcd));

    ESP_ERROR_CHECK(
        hd44780_control(&lcd, true, false, false)
    );

    ESP_ERROR_CHECK(hd44780_clear(&lcd));

    ESP_LOGI(TAG, "Display inicializado!");

    return ESP_OK;
}

// Escreve texto no display
esp_err_t lcd_print(const char *text)
{
    if (text == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    return hd44780_puts(&lcd, text);
}

// Limpa o display
esp_err_t lcd_clear(void)
{
    return hd44780_clear(&lcd);
}

// Posiciona o cursor
esp_err_t lcd_set_cursor(uint8_t col, uint8_t row)
{
    return hd44780_gotoxy(&lcd, col, row);
}