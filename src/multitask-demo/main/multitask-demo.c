/**
 * @file  multitask-demo.c
 * @brief FreeRTOS multitasking example for ESP32.
 *
 * @note
 *   - Demonstrates creation of multiple tasks.
 *   - Demonstrates task scheduling.
 *   - Demonstrates independent task periods.
 *   - Uses GPIO toggling for timing visualization.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/* GPIO definitions */
#define LED_TASK_1    (2U)
#define LED_TASK_2    (4U)

/**
 * @brief Task 1.
 *
 * Fast periodic task.
 */
static void TaskFast(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        gpio_set_level(LED_TASK_1, 1);

        printf("[TaskFast] Running on core %d\n", xPortGetCoreID());

        vTaskDelay(pdMS_TO_TICKS(100));

        gpio_set_level(LED_TASK_1, 0);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Task 2.
 *
 * Slow periodic task.
 */
static void TaskSlow(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        gpio_set_level(LED_TASK_2, 1);

        printf("[TaskSlow] Running on core %d\n", xPortGetCoreID());

        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(LED_TASK_2, 0);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Application entry point.
 */
void app_main(void)
{
    /* Configure GPIOs */
    gpio_reset_pin(LED_TASK_1);
    gpio_set_direction(LED_TASK_1, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_TASK_2);
    gpio_set_direction(LED_TASK_2, GPIO_MODE_OUTPUT);

    /* Create fast task */
    (void)xTaskCreate(
        TaskFast,
        "TaskFast",
        2048,
        NULL,
        2,
        NULL);

    /* Create slow task */
    (void)xTaskCreate(
        TaskSlow,
        "TaskSlow",
        2048,
        NULL,
        1,
        NULL);
}