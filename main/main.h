/**
 * @author Christopher Jones christopher.jones.wa@gmail.com
 * @date 2023-05-28
 * @file main.h
 *
 * @version 0.1
 * @copyright Copyright c 2023
 *
 */

#pragma once
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

/**
 * @brief Definitions for the LilyGo T-HMI board
 *
 */

// Using SPI2 as it also supports octal modes on some targets
#define LCD_HOST SPI2_HOST

// Transmit extra lines to speed up the display, number is divisible by LCD_V_RES 240
#define PARALLEL_LINES 16

// The SPI clock speed currently 10 MHz, 20 MHz also works
#define PXL_CLK_HZ 20 * 1000 * 1000

// The pixel number in horizontal and vertical
#define LCD_H_RES 320
#define LCD_V_RES 240

// Bit number used to represent command and parameter
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_PIN_PXL_CLK 8
#define LCD_PIN_CS 6
#define LCD_PIN_DC 7
#define LCD_PIN_RST GPIO_NUM_NC
#define LCD_PIN_BKLT 38

// Pins for the LCD
#define BKLT_ON_LVL 1
#define BKLT_OFF_LVL !BKLT_ON_LVL
#define LCD_PIN_DATA0 48 /*!< for 1-line SPI, this also refereed as MOSI */
#define LCD_PIN_DATA1 47
#define LCD_PIN_DATA2 39
#define LCD_PIN_DATA3 40
#define LCD_PIN_DATA4 41
#define LCD_PIN_DATA5 42
#define LCD_PIN_DATA6 45
#define LCD_PIN_DATA7 46

// NPower and enable pins
#define PWR_EN_PIN 10
#define PWR_ON_PIN 14

// Battery ADC and button pins
#define BAT_ADC_PIN 5
#define BUTTON1_PIN 0
#define BUTTON2_PIN 21

// For the xpt2046 SPI
#define TOUCH_SCLK_PIN 1
#define TOUCH_MISO_PIN 4
#define TOUCH_MOSI_PIN 3
#define TOUCH_CS_PIN 2
#define TOUCH_IRQ_PIN 9

#define BLK_SIZE 80
#define Y_BLKS (LCD_V_RES / BLK_SIZE)
#define X_BLKS (LCD_H_RES / BLK_SIZE)
#define NUM_BLKS (X_BLKS * Y_BLKS)
#define R_MID 0x10
#define G_MID 0x20
#define B_MID 0x10

void init_lcd(esp_lcd_panel_io_handle_t *io_handle,
              esp_lcd_panel_handle_t *panel_handle)
{
    ESP_LOGI("init_lcd", "Initializing beginning...");

    // Power enable and on pins
    gpio_set_direction(PWR_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(PWR_ON_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(PWR_EN_PIN, 1);
    gpio_set_level(PWR_ON_PIN, 1);

    // Backlight
    gpio_set_direction(LCD_PIN_BKLT, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_PIN_BKLT, BKLT_ON_LVL);
    ESP_LOGI("LCD", "Backlight on");

    // Configure the spi bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_PIN_PXL_CLK,
        .data0_io_num = LCD_PIN_DATA0,
        .data1_io_num = LCD_PIN_DATA1,
        .data2_io_num = LCD_PIN_DATA2,
        .data3_io_num = LCD_PIN_DATA3,
        .data4_io_num = LCD_PIN_DATA4,
        .data5_io_num = LCD_PIN_DATA5,
        .data6_io_num = LCD_PIN_DATA6,
        .data7_io_num = LCD_PIN_DATA7,
        .max_transfer_sz = BLK_SIZE * BLK_SIZE * 2 + 8,
        .flags = SPICOMMON_BUSFLAG_OCTAL};

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Configure the lcd_panel interface
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .cs_gpio_num = LCD_PIN_CS,
        .pclk_hz = PXL_CLK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 3,
        .flags.octal_mode = 1,
        .trans_queue_depth = 10,
    };

    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, io_handle));

    
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };

    // Initialize the LCD configuration
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(*io_handle, &panel_config, panel_handle));

    // esp_lcd_panel_handle_t panel_handle = init_lcd();

    ESP_LOGI("main", "LCD screen initialized.");

    // Reset the display
    esp_err_t err = esp_lcd_panel_reset(*panel_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_reset failed: %s", esp_err_to_name(err));
        abort();
    }

    // Initialize LCD panel
    err = esp_lcd_panel_init(*panel_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_init failed: %s", esp_err_to_name(err));
        abort();
    }

    // Turn on the screen
    err = esp_lcd_panel_disp_on_off(*panel_handle, true);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_disp_on_off failed: %s", esp_err_to_name(err));
        abort();
    }

    // Swap x and y axis (This sets the screen to landscape mode)
    err = esp_lcd_panel_swap_xy(*panel_handle, true);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_swap_xy failed: %s", esp_err_to_name(err));
        abort();
    }

    ESP_LOGI("init_lcd", "Initialization complete.");
}