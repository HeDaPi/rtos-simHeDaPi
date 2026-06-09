/**
 * @file rx-mcu.c
 * @brief UDP receiver example using ESP-IDF and FreeRTOS.
 *
 * This application connects an ESP32 to a WiFi network and listens
 * for incoming UDP packets from a remote node.
 *
 * Target:
 * - ESP32
 * - ESP-IDF 6.0
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "nvs_flash.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"

#define WIFI_SSID                  "Wokwi-GUEST"
#define WIFI_PASSWORD              ""

#define LOCAL_UDP_PORT             (3333U)

#define RX_TASK_STACK_SIZE         (4096U)
#define RX_TASK_PRIORITY           (5U)

#define WIFI_CONNECTION_TIMEOUT_S  (15U)

static const char* TAG = "UDP_RX";

/**
 * @brief Initializes WiFi in station mode.
 */
static void WifiInit(void)
{
    wifi_init_config_t wifiInitCfg = WIFI_INIT_CONFIG_DEFAULT();

    esp_netif_t* wifiNetif;

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifiNetif = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitCfg));

    wifi_config_t wifiConfig =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        }
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifiConfig));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    ESP_ERROR_CHECK(
        esp_wifi_connect());

    ESP_LOGI(
        TAG,
        "Connecting to WiFi...");

    for (uint32_t timeout = 0U;
         timeout < WIFI_CONNECTION_TIMEOUT_S;
         timeout++)
    {
        esp_netif_ip_info_t ipInfo;

        if (esp_netif_get_ip_info(
                wifiNetif,
                &ipInfo) == ESP_OK)
        {
            if (ipInfo.ip.addr != 0U)
            {
                ESP_LOGI(
                    TAG,
                    "Connected. IP: " IPSTR,
                    IP2STR(&ipInfo.ip));

                return;
            }
        }

        ESP_LOGI(
            TAG,
            "Waiting for IP address...");

        vTaskDelay(pdMS_TO_TICKS(1000U));
    }

    ESP_LOGE(
        TAG,
        "Failed to obtain IP address");
}

/**
 * @brief Receives UDP packets and prints their contents.
 *
 * @param[in] pvParameters Unused task parameter.
 */
static void RxTask(void* pvParameters)
{
    int32_t socketFd;
    int32_t receivedBytes;

    struct sockaddr_in localAddress;
    struct sockaddr_in sourceAddress;

    socklen_t sourceAddressLength;

    char receiveBuffer[128];

    (void)pvParameters;

    memset(
        &localAddress,
        0,
        sizeof(localAddress));

    localAddress.sin_family = AF_INET;

    localAddress.sin_port =
        htons(LOCAL_UDP_PORT);

    localAddress.sin_addr.s_addr =
        htonl(INADDR_ANY);

    socketFd = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_IP);

    if (socketFd < 0)
    {
        ESP_LOGE(
            TAG,
            "Failed to create UDP socket");

        vTaskDelete(NULL);
    }

    if (bind(
            socketFd,
            (struct sockaddr*)&localAddress,
            sizeof(localAddress)) < 0)
    {
        ESP_LOGE(
            TAG,
            "Failed to bind UDP socket");

        (void)close(socketFd);

        vTaskDelete(NULL);
    }

    ESP_LOGI(
        TAG,
        "Listening on UDP port %u",
        (unsigned int)LOCAL_UDP_PORT);

    for (;;)
    {
        sourceAddressLength =
            sizeof(sourceAddress);

        receivedBytes = recvfrom(
            socketFd,
            receiveBuffer,
            sizeof(receiveBuffer) - 1U,
            0,
            (struct sockaddr*)&sourceAddress,
            &sourceAddressLength);

        if (receivedBytes < 0)
        {
            ESP_LOGE(
                TAG,
                "Failed to receive UDP packet");

            continue;
        }

        receiveBuffer[receivedBytes] = '\0';

        ESP_LOGI(
            TAG,
            "RX <- %s from %s:%u",
            receiveBuffer,
            inet_ntoa(sourceAddress.sin_addr),
            ntohs(sourceAddress.sin_port));
    }
}

/**
 * @brief Application entry point.
 */
void app_main(void)
{
    BaseType_t taskStatus;

    esp_err_t status;

    ESP_LOGI(
        TAG,
        "UDP receiver starting");

    status = nvs_flash_init();

    if ((status == ESP_ERR_NVS_NO_FREE_PAGES) ||
        (status == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase());

        status = nvs_flash_init();
    }

    ESP_ERROR_CHECK(status);

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
    else
    {
        ESP_LOGI(
            TAG,
            "Reception task created");
    }
}