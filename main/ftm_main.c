
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_err.h"
#include "sdkconfig.h"

 
// Wifi Credentials
const char* SSID = "ssid";
const char* PWD = "pwd";
uint8_t channel = 1;
uint8_t bw = 40; // 20 or 40
 
wifi_config_t g_ap_config = {
    .ap.max_connection = 4,
    .ap.authmode = WIFI_AUTH_WPA2_PSK,
    .ap.ftm_responder = true
};
 
static const char *TAG_AP = "ftm_ap"; 
 
static void event_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG_AP, "SoftAP started with FTM Responder support");
    } else if (event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG_AP, "SoftAP stopped");
    }
}
 
static void wifi_ap()
{
    // Check if Wifi modules are ready/functional
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    strlcpy((char*) g_ap_config.ap.ssid, SSID, strlen(SSID) + 1);
    strlcpy((char*) g_ap_config.ap.password, PWD, strlen(PWD) + 1);
 
    // SoftAP Wifi configurations

    g_ap_config.ap.channel = channel;

    
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &g_ap_config));

    if (bw == 40) {
        esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_BW_HT40); // SoftAP Bandwidth: 40 MHz
    } else {
        esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_BW_HT20); // SoftAP Bandwidth: 20 MHz
    }
    
    ESP_LOGI(TAG_AP, "Starting SoftAP with FTM Responder support, SSID - %s, Password - %s, Primary Channel - %d, Bandwidth - %dMHz",
             SSID, PWD, channel, bw
    );
 
    // Start the Wifi modules
    ESP_ERROR_CHECK(esp_wifi_start());
}
 
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
 

    // Start the SoftAP and keep the code looping to host the SoftAP forever
    wifi_ap();
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // printf(xPortGetFreeHeapSize());
    }
}