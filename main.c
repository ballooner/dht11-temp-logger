#include <stdio.h>
#include <lgpio.h>

#define GPIO_PIN 17

enum ERR_E
{
	GPIO_OPEN_FAIL = -1,
	GPIO_CLAIM_OUTPUT_FAIL = -2,
	GPIO_CLAIM_INPUT_FAIL = -3,
};

// Send the start signal to the DHT11
// Returns:
// 0 on success
// -1 on failure
int sendStartSignal(int handle, int flags, int pin);
// Read the signal sent by the DHT11
// Returns:
// 0 on success
// -1 on failure
int readSignal(int handle, int flags, int pin);

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
		retVal = GPIO_CLAIM_OUTPUT_FAIL;
		goto cleanup_claim_fail;
	}

	// Read the signal
	if (readSignal(gpioHandle, flags, GPIO_PIN) < 0)
	{
		retVal = GPIO_CLAIM_INPUT_FAIL;
		goto cleanup_claim_fail;
	}
	
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

	// Free pin
	lgGpioFree(handle, pin);

	return 0;
}

int readSignal(int handle, int flags, int pin)
{
	

	return 0;
}
