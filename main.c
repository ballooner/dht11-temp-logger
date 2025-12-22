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
	int 	lastState;
	uint8_t  intHumidity;
	uint8_t  decHumidity;
	uint8_t  intTemp;
	uint8_t  decTemp;
} dht11_t;


// Function Definitions
dht11_t* dht11_create(void);
void dht11_destroy(dht11_t* sensor);
error_e dht11_read_loop(dht11_t* sensor);


int main(void)
{
	dht11_t* sensor = dht11_create();

	error_e retVal;
	retVal = dht11_read_loop(sensor);

	dht11_destroy(sensor);

	return retVal;
}

dht11_t* dht11_create(void)
{
	dht11_t* sensor = malloc(sizeof(dht11_t));

	sensor->lastState = 1; 
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

error_e dht11_read_loop(dht11_t* sensor)
{
	// Open the chip
	int handle = lgGpiochipOpen(0);
	if (handle < 0) 
	{
		LOG_MESSAGE("Error: Failure to open GPIO chip");
		return ERROR_GPIO_OPEN_FAIL;
	}

	while (1)
	{
		// Send start signal
		int retVal = lgGpioClaimOutput(handle, LG_SET_PULL_NONE, GPIO_PIN,  0);
		if (retVal < 0) 
		{
			LOG_MESSAGE("Error: Claiming GPIO as output failed.");
			return ERROR_GPIO_CLAIM_FAIL;
		}

		lguSleep(.018);
		
		retVal = lgGpioWrite(handle, GPIO_PIN, 1); 
		if (retVal < 0)
		{
			LOG_MESSAGE("Error: Writing to GPIO failed.");
			return ERROR_GPIO_WRITE_FAIL;
		}

		sensor->lastState = 1;
		
		// Read data
		retVal = lgGpioClaimInput(handle, LG_SET_PULL_NONE, GPIO_PIN);
		if (retVal < 0) 
		{
			LOG_MESSAGE("Error: Claiming GPIO as input failed.");
			return ERROR_GPIO_CLAIM_FAIL;
		}

		int bit = 0;
		uint8_t iter = 0;
		
		int currState = 1;
		while (retVal >= 0 && bit < 40)
		{
			if (currState == 0 && sensor->lastState == 1)
			{
				sensor->lastState = 0;
				iter = 0;
			} else if (currState == 1 && sensor->lastState == 0)
			{
				sensor->lastState = 1;
				iter = 0;
			}


			iter += 1;
			usleep(1);
		}
	}

	

	return SUCCESS;
}
