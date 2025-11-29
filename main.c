#include <stdio.h>
#include <lgpio.h>

#define GPIO_PIN 17

enum ERR_E
{
	GPIO_OPEN_FAIL = -1,
	GPIO_CLAIM_FAIL = -2,
	GPIO_ALERT_FAIL = -3,
};

uint64_t previous_timestamp;

// Send the start signal to the DHT11
// Returns:
// 0 on success
// -1 on failure
int sendStartSignal(int handle, int flags, int pin);
void readSignal(int num_alerts, lgGpioAlert_p alerts, void *userdata);

int main(void)
{
	int retVal = 0; // Program return value

	// Set up basic GPIO use
	int gpioHandle = lgGpiochipOpen(0);
	if (gpioHandle < 0) 
	{
		retVal = GPIO_OPEN_FAIL;
		goto cleanup_open_fail;
	}

	// Set on-board pull-up resistor to off
	int flags = LG_SET_PULL_NONE;
	
	// Send the start signal
	if (sendStartSignal(gpioHandle, flags, GPIO_PIN) < 0)
	{
		retVal = GPIO_CLAIM_FAIL;
		goto cleanup_claim_fail;
	}
	
	// Read the signal
	lgGpioSetAlertsFunc(gpioHandle, GPIO_PIN, readSignal, 0);

	previous_timestamp = lguTimestamp();
	if (lgGpioClaimAlert(gpioHandle, LG_SET_PULL_NONE, LG_BOTH_EDGES, GPIO_PIN, -1) < 0) 
	{
		retVal = GPIO_ALERT_FAIL;
		goto cleanup_claim_fail;
	}

	lguSleep(10);
	
	// Program cleanup labels
	cleanup_claim_fail:
	lgGpiochipClose(gpioHandle);
	cleanup_open_fail:
	return retVal;
}

int sendStartSignal(int handle, int flags, int pin)
{
	// Set GPIO pin value to 0
	if (lgGpioClaimOutput(handle, flags, pin, 0) < 0)
	{
		return -1;
	}

	// Sleep for 22ms to so DHT11 can register our signal
	// (datasheet says 18ms so I added +4ms to be sure)
	lguSleep(.022);

	// Set pin back to high
	if (lgGpioWrite(handle, pin, 1) < 0)
	{
		return -1;
	}

	// Free pin
	lgGpioFree(handle, pin);
	
	return 0;
}

void readSignal(int num_alerts, lgGpioAlert_p alerts, void *userdata)
{
	for (int i = 0; i < num_alerts; i++)
	{
		printf("Timestamp: %"PRIu64", Level: %d, Time difference: %"PRIu64" (%d of %d)\n",
			       	alerts[i].report.timestamp, alerts[i].report.level, (alerts[i].report.timestamp - previous_timestamp) / 1000, i + 1, num_alerts);
	}

	previous_timestamp = alerts[num_alerts - 1].report.timestamp;
}
