#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"


// не используем выравнивание износа
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

//монтирует раздел partname в путь mountpoint
int mount_partition(const char *partname, const char *mountpoint)
{
    const esp_vfs_fat_mount_config_t mount_config = {
		.max_files = 4,					//максимальное кол-во одновременно открытых файлов
		.format_if_mount_failed = true,	//форматировать раздел, если он еще не существует
		.allocation_unit_size = 0		//размер кластера равен размеру сектора
	};
	esp_err_t err;

	//монтируем раздел для чтения и записи
	err = esp_vfs_fat_spiflash_mount_rw_wl(mountpoint, partname, &mount_config, &s_wl_handle);

	if (err != ESP_OK)
	{
		printf("Failed to mount FATFS (%s)\n", esp_err_to_name(err));
		return -1;
	}
    return 0;
}


// пример использования
int fat_test()
{
	if(mount_partition("files","/storage") < 0)
    {
        return -1;
    }

	FILE* fp; 
	fp = fopen("/storage/test.txt", "r");
	if(fp == NULL)
	{
		printf("File not found\n");
		return -1;
	}

	char buf[1024];
	int bytes = fread(buf,1,1024,fp);
	buf[bytes] = '\0';
	printf("File content: %s\n", buf);

	fclose(fp);

	printf("Done\n");
	return 0;
}