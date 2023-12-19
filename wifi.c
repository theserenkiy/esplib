#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

//флажок о том, что получен IP
volatile int has_ip = 0;

//переменная для хранения объекта сетевого интерфейса
esp_netif_t *netif;


typedef struct {
	char ssid[64];
	char passwd[64];
} wifi_data_t;

//данные подключения к Wi-Fi (SSID, пароль)
//заполняется при первом вызове wifi_init()
wifi_data_t wifi_data;

static void on_wifi_disconnect();
static void on_got_ip();

//функция проверки на подключенность к вайфаю
int is_wifi_ready(void)
{
	return has_ip;
}

//ожидание подключения к вайфаю
void wait_for_wifi()
{
	while(1)
	{
		if(is_wifi_ready())
			return;
		printf("Waiting for wifi...\n");
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

//инициализация подключения к вайфаю
int wifi_connect()
{
	//заполняем структуру wifi_config
	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config));
	//копируем ssid и пароль в соотв. поля структуры
	strncpy((char *)wifi_config.sta.ssid, wifi_data.ssid, strlen(wifi_data.ssid));
	strncpy((char *)wifi_config.sta.password, wifi_data.passwd, strlen(wifi_data.passwd));
	
	printf("Connecting to %s...\n", wifi_config.sta.ssid);

	//устанавливаем режим работы ESP с вайфай - "станция" (то есть ведомое устройство)
	//(ещё может быть "точка доступа" - тогда ESP сама раздаёт вайфай)
	esp_wifi_set_mode(WIFI_MODE_STA);
	
	//передаем вайфайному движку заполненный конфиг
	esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

	//запускаем вайфай
	esp_wifi_start();

	//когда вайфай запустится, будет вызвана callback-функция on_wifi_start()
	//дальнейший код подключения - внутри этой функции
	
	return 0;
}

//обработчик события "Wi-Fi запущен"
static void on_wifi_start(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data)
{
	printf("Wi-Fi started\n");

	//запускаем процедуру подключения к вайфаю
	esp_err_t err = esp_wifi_connect();
		
	printf("esp_wifi_connect() returns: %d\n", err);
	
	//проверяем что функция подключения не вернула ошибку
	if (err == ESP_ERR_WIFI_NOT_STARTED) {
		printf("Wi-Fi not started\n");
		return;
	}

	printf("Waiting for connection...\n");

	//когда произойдет подключение, будет вызвана callback-функция on_wifi_connect()
}

//обработчик события "Подключено к Wi-Fi"
static void on_wifi_connect(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data)
{
	printf("Wi-Fi connected\n");
	//подключение установлено. 
	//с этого момента запускается DHCP и пытается получить IP-адрес
	//Как только IP получен - будет вызвана callback-функция on_got_ip()
	//только после успешного получения IP мы можем начинать работать с сетью
}

//обработчик события "Отключено от Wi-Fi"
static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data)
{
	printf("******************************\nWi-Fi disconnected, trying to reconnect...\n");
	has_ip = 0;
	esp_wifi_stop();
	wifi_connect();
}

//обработчик события "IP получен"
static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	printf("Wi-Fi Got IP\n");
	has_ip = 1;
	ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
	int ip = event->ip_info.ip.addr;
	printf("Your ip: %d.%d.%d.%d\n", (ip & 0xff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);
}

//обработчик события "IP потерян"
static void on_lost_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	has_ip = 0;
	printf("Wi-Fi LOST IP\n");
}


//инициализация вайфая
esp_netif_t *wifi_init(char *ssid, char *passwd)
{
	//инициализация хранилища во флеш-памяти (нужно для кэширования параметров WiFi)
	nvs_flash_init();

	//инициализируем цикл обработчика событий
	esp_event_loop_create_default();

	//инициализируем сетевой интерфейс 
	esp_netif_init();

	//вписываем данные подключения в глобальную структуру wifi_data (объявлена в начале файла)
	sprintf(wifi_data.ssid, ssid);
	sprintf(wifi_data.passwd, passwd);

	//получаем дефолтный конфиг для вайфая
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	cfg.nvs_enable = 1;

	//инитим вайфай дефолтным конфигом
	esp_wifi_init(&cfg);
	
	//получаем дефолтный конфиг вайфайного сетевого интерфейса
	esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_STA();

	//создаем новый сетевой интерфейс для вайфай
	netif = esp_netif_new(&netif_config);

	//подключаем интерфейс вайфай-станции к сетевому интерфейсу
	esp_netif_attach_wifi_station(netif);

	//сбрасываем обработчики событий вайфай-станции
	esp_wifi_set_default_wifi_sta_handlers();

	//добавляем обработчики событий:

	//событие "Вайфай запущен"
	//данное событие будет обрабатываться функцией on_wifi_start() (см. выше)
	esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &on_wifi_start, NULL);

	//событие "Подключено к вайфай"
	esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connect, NULL);

	//событие "Отключено от вайфай"
	esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL);

	//событие "IP-адрес получен"
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL);

	//событие "IP-адрес потерян"
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &on_lost_ip, NULL);

	//устанавливаем тип носителя для переменных вайфая - оперативная память (RAM)
	esp_wifi_set_storage(WIFI_STORAGE_RAM);

	//esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_1M_L);

	//переходим в функцию подключения
	wifi_connect();

	return netif;
}