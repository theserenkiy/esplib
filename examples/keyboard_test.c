#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define KB_cols 3
#define KB_rows 4

static char keys_pressed[256];
static char keys[KB_cols][KB_rows] = {{'1','4','7','*'},{'2','5','8','0'},{'3','6','9','#'}};
static int col_pins[] = {27, 14, 12};
static int row_pins[] = {32, 33, 25, 26};
static int keystate[KB_cols][KB_rows];

static void initPins(int pins[], int n, gpio_mode_t mode)
{
	for(int num=0; num < n; num++)
	{
		gpio_reset_pin(pins[num]);
		gpio_set_direction(pins[num],mode);
		if(mode==GPIO_MODE_INPUT)
			gpio_set_pull_mode(pins[num],GPIO_PULLDOWN_ONLY);
		else
			gpio_set_level(pins[num],0);
	}
}


void keyboardSummator()
{
	initPins(col_pins, KB_cols, GPIO_MODE_OUTPUT);
	initPins(row_pins, KB_rows, GPIO_MODE_INPUT);

	int cur_col = 0;
	int rd_val;
	char key;
	uint32_t sum = 0;
	uint32_t operand;
	int press_count = 0;
	while(1)
	{
		for(int col=0; col < KB_cols; col++)
		{
			if(cur_col)
				gpio_set_level(cur_col,0);
			
			cur_col = col_pins[col];
			gpio_set_level(col_pins[col],1);
			vTaskDelay(10/portTICK_PERIOD_MS);

			for(int row=0; row < KB_rows; row++)
			{
				rd_val = gpio_get_level(row_pins[row]);
				if(keystate[col][row]==rd_val)
					continue;
				
				keystate[col][row] = rd_val;
				//printf("%d %d: %d\n",row,col,rd_val);
				if(rd_val){
					key = keys[col][row];
					if(key == '#')
					{
						if(!press_count)continue;
						sscanf(keys_pressed,"%lu",&operand);
						sum += operand;
						printf("Sum: %lu\n",sum);
						press_count = 0;
					}
					else if(key == '*')
					{
						sum = 0;
						press_count = 0;
					}
					else{
						keys_pressed[press_count] = key;
						keys_pressed[press_count+1] = 0;
						press_count++;

						printf("Operand: %s\n",keys_pressed);
					}
				}
				
			}
			vTaskDelay(10/portTICK_PERIOD_MS); 
		}
	}
}