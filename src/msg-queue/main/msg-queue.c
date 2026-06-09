/**
 * @file msg-queue.c
 * @brief FreeRTOS Queue communication example.
 *
 * Two tasks communicate using a message queue.
 *
 * Target:
 * - ESP32
 * - ESP-IDF 6.x
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#define QUEUE_LENGTH       (5U)
#define MESSAGE_SIZE       (32U)

#define TX_STACK_SIZE      (4096U)
#define RX_STACK_SIZE      (4096U)

#define TX_PRIORITY        (5U)
#define RX_PRIORITY        (5U)

static const char* TAG = "QUEUE_DEMO";

typedef struct
{
    uint32_t counter;
    char message[MESSAGE_SIZE];
} QueueMessage_t;

static QueueHandle_t gMessageQueue;

/**
 * @brief Producer task.
 */
static void ProducerTask(void* pvParameters)
{
    QueueMessage_t txMessage;

    (void)pvParameters;

    txMessage.counter = 0U;

    for (;;)
    {
        snprintf(
            txMessage.message,
            sizeof(txMessage.message),
            "MSG:%lu",
            (unsigned long)txMessage.counter);

        if (xQueueSend(
                gMessageQueue,
                &txMessage,
                pdMS_TO_TICKS(100U)) == pdPASS)
        {
            ESP_LOGI(
                TAG,
                "TX -> %s",
                txMessage.message);
        }
        else
        {
            ESP_LOGW(
                TAG,
                "Queue full, message discarded");
        }

        txMessage.counter++;

        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

/**
 * @brief Consumer task.
 */
static void ConsumerTask(void* pvParameters)
{
    QueueMessage_t rxMessage;

    (void)pvParameters;

    for (;;)
    {
        if (xQueueReceive(
                gMessageQueue,
                &rxMessage,
                portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(
                TAG,
                "RX <- %s (counter=%lu)",
                rxMessage.message,
                (unsigned long)rxMessage.counter);
        }
    }
}

/**
 * @brief Application entry point.
 */
void app_main(void)
{
    BaseType_t status;

    ESP_LOGI(TAG, "Application started");

    gMessageQueue = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(QueueMessage_t));

    if (gMessageQueue == NULL)
    {
        ESP_LOGE(
            TAG,
            "Failed to create queue");

        return;
    }

    ESP_LOGI(
        TAG,
        "Queue created successfully");

    status = xTaskCreate(
        ProducerTask,
        "ProducerTask",
        TX_STACK_SIZE,
        NULL,
        TX_PRIORITY,
        NULL);

    if (status != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create ProducerTask");

        return;
    }

    status = xTaskCreate(
        ConsumerTask,
        "ConsumerTask",
        RX_STACK_SIZE,
        NULL,
        RX_PRIORITY,
        NULL);

    if (status != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create ConsumerTask");

        return;
    }

    ESP_LOGI(
        TAG,
        "Tasks created successfully");
}