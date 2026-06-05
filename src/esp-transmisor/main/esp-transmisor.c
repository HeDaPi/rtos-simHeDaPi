/**
 * @file esp-transmisor.c
 * @brief UDP transmitter example using ESP-IDF and FreeRTOS.
 *
 * This application connects an ESP32 to a WiFi network and periodically
 * transmits UDP packets to a remote node.
 *
 * Educational objectives:
 * - FreeRTOS task creation.
 * - WiFi station mode configuration.
 * - UDP socket communication.
 * - Distributed systems concepts.
 * - Message-passing architectures.
 * - OCCAM/CSP communication model analogy.
 *
 * Target:
 * - ESP32
 * - ESP-IDF 6.x
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"

#define WIFI_SSID              "Wokwi-GUEST"
#define WIFI_PASSWORD          ""

#define RECEIVER_IP_ADDRESS    "10.0.2.15"
#define RECEIVER_UDP_PORT      (3333)

#define TX_TASK_STACK_SIZE     (4096U)
#define TX_TASK_PRIORITY       (5U)

static const char* TAG = "UDP_TX";

/**
 * @brief Connects the device to the configured WiFi network.
 */
static void WifiInit(void)
{
    wifi_init_config_t wifiInitCfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    (void)esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitCfg));

    wifi_config_t wifiConfig =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifiConfig));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG, "WiFi connection started");
}

/**
 * @brief Periodically transmits UDP messages.
 *
 * @param[in] pvParameters Unused task parameter.
 */
static void TxTask(void* pvParameters)
{
    int32_t socketFd;
    struct sockaddr_in destinationAddress;

    uint32_t counter = 0U;
    char messageBuffer[64];

    (void)pvParameters;

    memset(&destinationAddress, 0, sizeof(destinationAddress));

    destinationAddress.sin_family = AF_INET;
    destinationAddress.sin_port =
        htons(RECEIVER_UDP_PORT);

    destinationAddress.sin_addr.s_addr =
        inet_addr(RECEIVER_IP_ADDRESS);

    socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (socketFd < 0)
    {
        ESP_LOGE(TAG, "Failed to create UDP socket");

        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "UDP socket created");

    for (;;)
    {
        (void)snprintf(
            messageBuffer,
            sizeof(messageBuffer),
            "MSG:%lu",
            (unsigned long)counter);

        (void)sendto(
            socketFd,
            messageBuffer,
            strlen(messageBuffer),
            0,
            (struct sockaddr*)&destinationAddress,
            sizeof(destinationAddress));

        ESP_LOGI(
            TAG,
            "TX -> %s",
            messageBuffer);

        counter++;

        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

/**
 * @brief Application entry point.
 */
void app_main(void)
{
    BaseType_t taskStatus;

    WifiInit();

    taskStatus = xTaskCreate(
        TxTask,
        "TxTask",
        TX_TASK_STACK_SIZE,
        NULL,
        TX_TASK_PRIORITY,
        NULL);

    if (taskStatus != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create transmission task");
    }
}