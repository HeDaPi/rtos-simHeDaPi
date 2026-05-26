/**
 * @file  starvation-demo.c
 * @brief FreeRTOS starvation example for ESP32.
 *
 * @note
 *   - Demonstrates task starvation.
 *   - Demonstrates priority-based scheduling.
 *   - Demonstrates CPU monopolization.
 *   - Demonstrates the importance of yielding.
 *   - Compatible with ESP-IDF 6.0.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/* GPIO definitions */
#define LED_HOG       (2U)
#define LED_VICTIM    (4U)

/**
 * @brief High-priority CPU hogging task.
 *
 * This task intentionally:
 *   - never blocks,
 *   - never delays,
 *   - never yields.
 *
 * As a consequence, lower-priority tasks
 * experience starvation.
 *
 * @param[in] pvParameters
 * Pointer to task parameters.
 */
static void TaskHog(void *pvParameters)
{
    volatile uint32_t counter = 0U;

    (void)pvParameters;

    gpio_set_level(LED_HOG, 1);

    while (1)
    {
        counter++;

        if ((counter % 10000000U) == 0U)
        {
            printf(
                "[TaskHog] Running... counter=%lu\n",
                (unsigned long)counter
            );
        }

        /**
         * Intentionally:
         *   - no vTaskDelay()
         *   - no taskYIELD()
         */
    }
}

/**
 * @brief Low-priority victim task.
 *
 * This task attempts to execute periodically,
 * but starvation prevents normal execution.
 *
 * @param[in] pvParameters
 * Pointer to task parameters.
 */
static void TaskVictim(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        gpio_set_level(LED_VICTIM, 1);

        printf(
            "[TaskVictim] I finally got CPU time!\n"
        );

        vTaskDelay(pdMS_TO_TICKS(100));

        gpio_set_level(LED_VICTIM, 0);

        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

/**
 * @brief Application entry point.
 */
void app_main(void)
{
    printf("\n");
    printf("====================================\n");
    printf("ESP32 FreeRTOS Starvation Demo\n");
    printf("====================================\n");

    /* Configure GPIOs */
    gpio_reset_pin(LED_HOG);
    gpio_set_direction(LED_HOG, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_VICTIM);
    gpio_set_direction(LED_VICTIM, GPIO_MODE_OUTPUT);

    /* Initialize GPIO states */
    gpio_set_level(LED_HOG, 0);
    gpio_set_level(LED_VICTIM, 0);
    
    /**
     * Create high-priority hogging task.
     */
    (void)xTaskCreate(
        TaskHog,
        "TaskHog",
        4096,
        NULL,
        10,
        NULL
    );

    /**
     * Create low-priority victim task.
     */
    (void)xTaskCreate(
        TaskVictim,
        "TaskVictim",
        4096,
        NULL,
        1,
        NULL
    );
}