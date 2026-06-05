/**
 * @file esp-receptor.c
 * @brief UDP receiver example using ESP-IDF and FreeRTOS.
 *
 * This application connects an ESP32 to a WiFi network and listens
 * for incoming UDP packets from a remote node.
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

#define LOCAL_UDP_PORT         (3333)

#define RX_TASK_STACK_SIZE     (4096U)
#define RX_TASK_PRIORITY       (5U)

static const char* TAG = "UDP_RX";

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
 * @brief Receives UDP packets and prints their contents.
 *
 * @param[in] pvParameters Unused task parameter.
 */
static void RxTask(void* pvParameters)
{
    int32_t socketFd;

    struct sockaddr_in localAddress;
    struct sockaddr_in sourceAddress;

    socklen_t sourceAddressLength;

    int32_t receivedBytes;

    char receiveBuffer[128];

    (void)pvParameters;

    memset(&localAddress, 0, sizeof(localAddress));

    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(LOCAL_UDP_PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (socketFd < 0)
    {
        ESP_LOGE(TAG, "Failed to create UDP socket");

        vTaskDelete(NULL);
    }

    if (bind(
            socketFd,
            (struct sockaddr*)&localAddress,
            sizeof(localAddress)) < 0)
    {
        ESP_LOGE(TAG, "Failed to bind UDP socket");

        (void)close(socketFd);

        vTaskDelete(NULL);
    }

    ESP_LOGI(
        TAG,
        "Listening on UDP port %u",
        (unsigned int)LOCAL_UDP_PORT);

    for (;;)
    {
        sourceAddressLength = sizeof(sourceAddress);

        receivedBytes = recvfrom(
            socketFd,
            receiveBuffer,
            sizeof(receiveBuffer) - 1,
            0,
            (struct sockaddr*)&sourceAddress,
            &sourceAddressLength);

        if (receivedBytes > 0)
        {
            receiveBuffer[receivedBytes] = '\0';

            ESP_LOGI(
                TAG,
                "RX <- %s",
                receiveBuffer);
        }
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
        RxTask,
        "RxTask",
        RX_TASK_STACK_SIZE,
        NULL,
        RX_TASK_PRIORITY,
        NULL);

    if (taskStatus != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create reception task");
    }
}