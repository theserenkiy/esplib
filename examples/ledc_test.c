#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits


static void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE, 
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 5000,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = 12,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void ledc_test(void)
{
    // Set the LEDC peripheral configuration
    ledc_init();

    while(1)
    {
        for(int i = 0; i < 100; i++)
        {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, i*80);
            // Update duty to apply the new value
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

            vTaskDelay(10/portTICK_PERIOD_MS);
        }

        for(int i = 99; i >= 0; i--)
        {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, i*80);
            // Update duty to apply the new value
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }

    
}
