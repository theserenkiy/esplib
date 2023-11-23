#include "time.h"
#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"

int syncTime()
{
	esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(
		2,
        ESP_SNTP_SERVER_LIST("time.windows.com", "pool.ntp.org" ) 
	);
    esp_netif_sntp_init(&config);
	int attempts = 10;
	for(int i = 0; i < attempts; i++)
	{
		printf("Trying to sync time: %d/%d\n", i, attempts);
		if(!esp_netif_sntp_sync_wait(pdMS_TO_TICKS(2000)))
			return 0;
	}
	return -1;
}

void getCurTime(char *str)
{
	time_t utime = time(NULL);
	struct tm *tm = gmtime(&utime);
	sprintf(str,"%d:%d:%d", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void timeTest()
{
    syncTime();

	time_t utime; 
	struct tm *tm;
	
	while(1)
	{
		utime = time(NULL);
		tm = gmtime(&utime);
		//printf("Current time: %d-%d-%d %d:%d:%d\n", tm->year, tm->mon, tm->mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("Current time: %d:%d:%d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}