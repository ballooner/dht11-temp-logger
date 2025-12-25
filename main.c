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
	ERROR_DHT_INIT_FAIL   = -1,
	ERROR_GPIO_OPEN_FAIL  = -2,
	ERROR_GPIO_CLAIM_FAIL = -3,
	ERROR_GPIO_WRITE_FAIL = -4,
	ERROR_GPIO_ALERT_FAIL = -5,
	ERROR_GPIO_FREE_FAIL  = -6,
} error_e;

typedef struct DHT11_DATA
{
	uint8_t  intHumidity;
	uint8_t  decHumidity;
	uint8_t  intTemp;
	uint8_t  decTemp;
} dht11_t;


// Function Definitions
dht11_t* dht11_init(void);
void dht11_free(dht11_t* sensor);
error_e dht11_read_loop(dht11_t* sensor);
void dht11_callback(int num_alerts, lgGpioAlert_p alerts, void* data);


int main(void)
{
	dht11_t* sensor = dht11_init();
	if (sensor == NULL)
	{
		LOG_MESSAGE("DHT11 struct initialization failed");
		return ERROR_DHT_INIT_FAIL;
	}

	int retVal = dht11_read_loop(sensor);
	if (retVal != SUCCESS)
	{
		LOG_MESSAGE("DHT11 read loop failed");
		return retVal;
	}

	dht11_free(sensor);

	return SUCCESS;
}

dht11_t* dht11_init(void)
{
	dht11_t* sensor = malloc(sizeof(dht11_t));

	sensor->intHumidity = 0;
	sensor->decHumidity = 0;
	sensor->intTemp = 0;
	sensor->decTemp = 0;

	return sensor;
}

void dht11_free(dht11_t* sensor)
{
	free(sensor);
}

error_e dht11_read_loop(dht11_t* sensor)
{
	int retVal = SUCCESS;

	// Open chip
	int chip = lgGpiochipOpen(0);
	if (chip < 0)
	{
		LOG_MESSAGE("GPIO chip failed to open");
		return ERROR_GPIO_OPEN_FAIL;
	}

	while (1)
	{
		// Send start signal
		if (lgGpioClaimOutput(chip, LG_SET_PULL_NONE, GPIO_PIN, 0) < 0)
		{
			LOG_MESSAGE("Failed to claim GPIO as output");
			retVal = ERROR_GPIO_CLAIM_FAIL;
			break;
		}
		
		sleep(.018);

		if (lgGpioWrite(chip, GPIO_PIN, 1) < 0)
		{
			LOG_MESSAGE("Failed to write to GPIO");
			retVal = ERROR_GPIO_WRITE_FAIL;
			break;
		}

		if (lgGpioFree(chip, GPIO_PIN) < 0) // Free pin so we can interpret the signal
		{
			LOG_MESSAGE("Failed to free GPIO");
			retVal = ERROR_GPIO_FREE_FAIL;
			break;
		}

		// Add callback function for signal change
		if (lgGpioSetAlertsFunc(chip, GPIO_PIN, dht11_callback, &sensor) < 0)
		{
			LOG_MESSAGE("Failed to set GPIO alert");
			retVal = ERROR_GPIO_ALERT_FAIL;
			break;
		}

		if (lgGpioClaimAlert(chip, LG_SET_PULL_NONE, LG_BOTH_EDGES, GPIO_PIN, -1) < 0)
		{
			LOG_MESSAGE("Failed to claim GPIO alert");
			retVal = ERROR_GPIO_ALERT_FAIL;
			break;
		}




		if (lgGpioFree(chip, GPIO_PIN) < 0) // Free pin to send the next start signal
		{
			LOG_MESSAGE("Failed to free GPIO");
			retVal = ERROR_GPIO_FREE_FAIL;
			break;
		}

		sleep(100);
	}

	return retVal;
}

void dht11_callback(int num_alerts, lgGpioAlert_p alerts, void* data)
{
	static uint64_t lastTimestamp = 0;
	uint64_t currTimestamp = alerts[0].report.timestamp;

	printf("Num events: %d, Alert[0] timestamp: %" PRIu64 "\n", num_alerts, currTimestamp);
}

