#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "application.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define TAG "main"

extern "C" void app_main(void)
{
        // ==================== 深度睡眠判断 ====================
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    if (wakeup_reason != ESP_SLEEP_WAKEUP_EXT1) {
        ESP_LOGI(TAG, "Not woken by card, entering deep sleep...");
        
        // 配置 GPIO 1 为 RTC 输入，启用内部下拉
        rtc_gpio_init(GPIO_NUM_1);
        rtc_gpio_set_direction(GPIO_NUM_1, RTC_GPIO_MODE_INPUT_ONLY);
        rtc_gpio_pullup_dis(GPIO_NUM_1);
        rtc_gpio_pulldown_en(GPIO_NUM_1);
        
        // 配置 EXT1 唤醒源：GPIO 1 高电平触发
        esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_1, ESP_EXT1_WAKEUP_ANY_HIGH);
        
        // 进入深度睡眠
        esp_deep_sleep_start();
    }
    
    // 被刷卡唤醒 → 正常启动
    ESP_LOGI(TAG, "Woken by card swipe, starting normal operation...");
    // ==================== 深度睡眠判断结束 ====================

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize and run the application
    auto& app = Application::GetInstance();
    app.Initialize();
    app.Run();  // This function runs the main event loop and never returns
}
