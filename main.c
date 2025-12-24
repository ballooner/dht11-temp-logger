#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <lgpio.h>
#include <math.h>

#define LOG_MESSAGE(MESSAGE) (fprintf(stderr, "File: %s, Line: %d, Error: %s\n", __FILE__, __LINE__, MESSAGE))

#define GPIO_PIN 17

// Data Definitions
typedef enum ERR_E
{
	SUCCESS 	      = 0,
	ERROR_GPIO_OPEN_FAIL  = -1,
	ERROR_GPIO_CLAIM_FAIL = -2,
	ERROR_GPIO_WRITE_FAIL = -3,
} error_e;

typedef struct DHT11_DATA
{
	int 	 chip;
	uint8_t  intHumidity;
	uint8_t  decHumidity;
	uint8_t  intTemp;
	uint8_t  decTemp;
} dht11_t;


// Function Definitions
error_e dht11_init(dht11_t* sensor);
error_e dht11_read_loop(dht11_t* sensor);
void dht11_callback(int num_alerts, lgGpioAlert_p alerts, void* data);


int main(void)
{
	error_e retVal;

	dht11_t sensor;
	retVal = dht11_init(&sensor);
	if (retVal < 0) return retVal;

	return retVal;
}

error_e dht11_init(dht11_t* sensor)
{
	sensor->chip = lgGpiochipOpen(0);
	if (sensor->chip < 0)
	{
		LOG_MESSAGE("Chip failed to open");
		return ERROR_GPIO_OPEN_FAIL;
	}
	sensor->intHumidity = 0;
	sensor->decHumidity = 0;
	sensor->intTemp = 0;
	sensor->decTemp = 0;

	return SUCCESS;
}
