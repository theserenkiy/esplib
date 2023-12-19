#include "wifi.c"
#include "time_sync.c"
#include "freertos/FreeRTOS.h"

typedef struct {
	int state;
	char time[10];
}log_row_t;

void wifi_test()
{
	wifi_init(WIFI_SSID, WIFI_PASS);
	wait_for_wifi();
	syncTime();

	char time[32];
	
	gpio_reset_pin(12);
	gpio_set_direction(12,GPIO_MODE_OUTPUT);
	gpio_set_drive_capability(12,GPIO_DRIVE_CAP_0);
	
	int laststate = 0, state = 0;
	
	log_row_t log[32];
	int log_cnt = 0;

	while(1)
	{
		state = is_wifi_ready();
		gpio_set_level(12,state);

		if(state != laststate)
		{
			log[log_cnt%32].state = state;
			getCurTime(log[log_cnt%32].time);
			log_cnt++;

			printf("********************************\n");
			for(int i=0; i < log_cnt%32;i++)
			{
				printf("%d:	%s\n",log[i].state,log[i].time);
			}

			printf("********************************\n");

			if(state && !laststate)
			{
				printf("WiFi ON\n");
			}
			else
			{
				printf("WiFi OFF\n");
			}
		}

		laststate = state;

		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}