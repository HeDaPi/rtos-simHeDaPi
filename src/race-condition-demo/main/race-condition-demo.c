/**
 * @file    race-condition-demo.c
 * @brief   Race condition demonstration using ESP-IDF v6.0.
 *
 * @details
 * This example demonstrates a classic race condition where two
 * FreeRTOS tasks concurrently increment the same shared variable
 * without synchronization.
 *
 * The expected final counter value is not reached because the
 * increment operation is not atomic.
 */

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define TASK_ITERATIONS    (100000U)
#define TASK_DELAY_MS      (1U)

/* Shared resource */
static volatile uint32_t g_sharedCounter = 0U;

/* Mutex protecting the shared resource */
static SemaphoreHandle_t g_counterMutex = NULL;

/**
 * @brief Task that increments the shared counter.
 *
 * @param pvParameters Pointer to task name string.
 */
static void CounterTask(void *pvParameters)
{
    const char *taskName = (const char *)pvParameters;

    uint32_t i;

    for (i = 0U; i < TASK_ITERATIONS; i++)
    {
        /*
         * Non-atomic operation:
         *
         * 1. Read
         * 2. Modify
         * 3. Write
         *
         * Context switching between these steps produces
         * the race condition.
         */
        if (xSemaphoreTake(g_counterMutex, portMAX_DELAY) == pdTRUE)
        {
            g_sharedCounter++;

            /*
             * Release the shared resource.
             */
            (void)xSemaphoreGive(g_counterMutex);
        }

        if ((i % 1000U) == 0U)
        {
            vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
        }
    }

    printf("%s finished\n", taskName);

    vTaskDelete(NULL);
}

/**
 * @brief Main application entry point.
 */
void app_main(void)
{
    printf("\n");
    printf("=====================================\n");
    printf("    ESP32 Mutex Protection Demo\n");
    printf("=====================================\n");

    /*
     * Expected result:
     *
     * 100000 + 100000 = 200000
     */
    printf("Expected final value: %u\n",
           (TASK_ITERATIONS * 2U));

    /*
     * Create mutex used to protect the shared counter.
     */
    g_counterMutex = xSemaphoreCreateMutex();

    if (g_counterMutex == NULL)
    {
        printf("Failed to create mutex\n");
        return;
    }

    xTaskCreate(
        CounterTask,
        "Task_A",
        2048U,
        (void *)"Task_A",
        5U,
        NULL);

    xTaskCreate(
        CounterTask,
        "Task_B",
        2048U,
        (void *)"Task_B",
        5U,
        NULL);

    /*
     * Allow tasks to finish.
     */
    vTaskDelay(pdMS_TO_TICKS(5000U));

    printf("Actual final value: %lu\n",
           (unsigned long)g_sharedCounter);

    /*
     * Cleanup.
     */
    vSemaphoreDelete(g_counterMutex);
}