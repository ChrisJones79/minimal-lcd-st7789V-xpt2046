/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portable.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "pretty_effect.h"
#include "main.h"

/**
 * @brief Static function declarations
 *
 *
 */
static void get_two_colors(uint16_t color[]);

static uint16_t *s_blks[NUM_BLKS];

static void random_4x3_blocks(esp_lcd_panel_handle_t panel_handle, uint8_t fps, uint8_t nframes);

/**
 * @brief The app_main function is the entry point of the program.
 *
 */
void app_main(void)
{
    ESP_LOGD("main", "app_main");

    // Initialize the lcd screen
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_handle_t panel_handle = NULL;

    init_lcd(&io_handle, &panel_handle);

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

/**
 * @brief Static function definitions
 *
 */

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

/**
 * @brief The random_4x3_blocks function takes a pointer to the lcd screen handle,
 * the number of frames per second, and the number of frames to display.
 * The function draws 4x3 blocks on the screen, grabbing two random colors at a time.
 * The function then draws the blocks on the screen.
 *
 */
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
