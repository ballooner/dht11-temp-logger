#include <stdlib.h>
#include <stdio.h>
#include <lgpio.h>
#include <math.h>

#define GPIO_PIN 17

// Data Definitions
typedef enum ERR_E
{
	SUCCESS 	      = 0,
	ERROR_GPIO_OPEN_FAIL  = -1,
	ERROR_GPIO_CLAIM_FAIL = -2,
	ERROR_GPIO_ALERT_FAIL = -3,
} error_e;

typedef struct DHT11_DATA
{
	int 	lastValue;
	uint8_t intHumidity;
	uint8_t decHumidity;
	uint8_t intTemp;
	uint8_t decTemp;
} dht11_t;


// Function Definitions
dht11_t* dht11_create(void);
void dht11_destroy(dht11_t* sensor);
error_e dht11_read(dht11_t* sensor);


int main(void)
{
	dht11_t* sensor = dht11_create();

	while (dht11_read(sensor) == SUCCESS) lguSleep(.01); 

	dht11_destroy(sensor);

	return SUCCESS;
}

dht11_t* dht11_create(void)
{
	dht11_t* sensor = malloc(sizeof(dht11_t));

	sensor->lastValue = 1; // DHT11 should start at high voltage because of the pull-up resistor
	sensor->intHumidity = 0;
	sensor->decHumidity = 0;
	sensor->intTemp = 0;
	sensor->decTemp = 0;

	return sensor;
}

void dht11_destroy(dht11_t* sensor)
{
	free(sensor);
}

error_e dht11_read(dht11_t* sensor)
{
	// Open the chip
	int handle = lgGpioChipOpen(0);

	// Send start signal
	lgGpioClaimOutput(handle, LG_SET_PULL_NONE, GPIO_PIN,  0);
	lguSleep(.018);
	lgGpioWrite(handle, GPIO_PIN, 1); 
	

	return SUCCESS;
}
