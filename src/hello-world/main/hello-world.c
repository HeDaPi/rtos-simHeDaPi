/**
 * @file  hello-world.c
 * @brief Basic FreeRTOS Hello World example for ESP32.
 *
 * @note
 *   - Demonstrates how to create a task.
 *   - Demonstrates periodic execution using vTaskDelay().
 *   - Compatible with ESP-IDF.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Task function.
 *
 * This task runs forever and prints a message every second.
 */
static void HelloTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        printf("Hello World from FreeRTOS task!\n");

        /* Delay task execution for 1000 ms */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Application entry point.
 */
void app_main(void)
{
    /**
     * @brief Create HelloTask.
     *
     * @param Task function
     * @param Task name
     * @param Stack size
     * @param Parameters
     * @param Priority
     * @param Task handle
     */
    (void)xTaskCreate(
        HelloTask,
        "HelloTask",
        2048,
        NULL,
        1,
        NULL);
}