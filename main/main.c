/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "pretty_effect.h"
#include "main.h"

/**
 * @brief The get_two_colors function takes a pointer to a 16-bit integer,
 * and sets the value of the integer to a random color.
 * The color is chosen from the list of colors in the global color_list array.
 * The get_two_colors function is called by the get_two_random_colors function, which is used to choose two random colors for the game.
 *
 */

static void get_two_colors(uint16_t color[])
{
    uint32_t rand_colors = esp_random();
    color[0] = (rand_colors >> 16) & 0xffff;
    color[1] = rand_colors & 0xffff;
}

static uint16_t *s_blks[NUM_BLKS];
static void random_4x3_blocks(esp_lcd_panel_handle_t panel_handle, uint8_t fps, uint8_t nframes)
{
    uint16_t colors[2];

    int xofs[][2] = {{0, BLK_SIZE},
                     {BLK_SIZE, 2 * BLK_SIZE},
                     {2 * BLK_SIZE, 3 * BLK_SIZE},
                     {3 * BLK_SIZE, 4 * BLK_SIZE}};

    int yofs[][2] = {{0, BLK_SIZE},
                     {BLK_SIZE, 2 * BLK_SIZE},
                     {2 * BLK_SIZE, 3 * BLK_SIZE}};

    int index = 0;

    // Draw blocks and get more colors
    for (int y = 0; y < Y_BLKS; y++)
    {
        for (int x = 0; x < X_BLKS; x += 2)
        {
            // Get two colors
            get_two_colors(&colors[0]);

            // Calculate the index
            index = 4 * y + x;

            // Copy the colors to the block
            memset(s_blks[index], colors[index % 2], sizeof(uint16_t) * BLK_SIZE * BLK_SIZE);
            memset(s_blks[index + 1], colors[index % 2 + 1], sizeof(uint16_t) * BLK_SIZE * BLK_SIZE);

            // Draw the two blocks
            esp_err_t err = esp_lcd_panel_draw_bitmap(panel_handle, xofs[x][0], yofs[y][0], xofs[x][1], yofs[y][1], s_blks[index]);
            if (err != ESP_OK)
            {
                ESP_LOGE("random_4x3_blocks", "esp_lcd_panel_draw_bitmap failed with %d", err);
                return;
            }

            err = esp_lcd_panel_draw_bitmap(panel_handle, xofs[x + 1][0], yofs[y][0], xofs[x + 1][1], yofs[y][1], s_blks[index + 1]);
            if (err != ESP_OK)
            {
                ESP_LOGE("random_4x3_blocks", "esp_lcd_panel_draw_bitmap failed with %d", err);
                return;
            }
        }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void app_main(void)
{
    ESP_LOGD("main", "app_main");

    // Initialize the lcd screen
    esp_lcd_panel_handle_t panel_handle = init_lcd();

    ESP_LOGI("main", "LCD screen initialized.");

    // Reset the display
    esp_err_t err = esp_lcd_panel_reset(panel_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_reset failed: %s", esp_err_to_name(err));
        abort();
    }

    // Initialize LCD panel
    err = esp_lcd_panel_init(panel_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_init failed: %s", esp_err_to_name(err));
        abort();
    }

    // Turn on the screen
    err = esp_lcd_panel_disp_on_off(panel_handle, true);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_disp_on_off failed: %s", esp_err_to_name(err));
        abort();
    }

    // Swap x and y axis (This sets the screen to landscape mode)
    err = esp_lcd_panel_swap_xy(panel_handle, true);
    if (err != ESP_OK)
    {
        ESP_LOGE("main", "esp_lcd_panel_swap_xy failed: %s", esp_err_to_name(err));
        abort();
    }

    // Need to mirror the screen to display correctly
    // err = esp_lcd_panel_set_gap(panel_handle, -20, 20);
    // if (err != ESP_OK)
    // {
    //     ESP_LOGE("main", "esp_lcd_panel_set_gap failed: %s", esp_err_to_name(err));
    //     abort();
    // }

    // Allocate memory for 4x3 blocks
    for (int i = 0; i < NUM_BLKS; i++)
    {
        s_blks[i] = heap_caps_malloc(BLK_SIZE * BLK_SIZE * sizeof(uint16_t), MALLOC_CAP_DMA);
        if (s_blks[i] == NULL)
        {
            ESP_LOGE("main", "heap_caps_malloc failed");
            abort();
        }
    }

    ESP_LOGI("main", "Memory allocated.");

    // Start and rotate
    while (1)
    {
        random_4x3_blocks(panel_handle, 30, 45);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
