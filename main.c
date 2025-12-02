#include <stdio.h>
#include <lgpio.h>
#include <math.h>

#define GPIO_PIN 17

enum ERR_E
{
	GPIO_OPEN_FAIL  = -1,
	GPIO_CLAIM_FAIL = -2,
	GPIO_ALERT_FAIL = -3,
};

int bits[40];
uint64_t previousTimestamp  = -1;
int bitsReceived = 0;

// Send the start signal to the DHT11
// Returns:
// 0 on success
// -1 on failure
int sendStartSignal(int handle, int flags, int pin);
// Reads the DHT11 signal and stores the bits in a bits array of length 40
void readSignal(int num_alerts, lgGpioAlert_p alerts, void *userdata);
// Converts an 8-bit binary number that is stored in an array to decimal. Range is n..n+7
// Assumes the MSB is at the lowest index and LSB is highest
uint8_t byteBin2Dec(int* arr, int n);

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

	if (lgGpioClaimAlert(gpioHandle, LG_SET_PULL_NONE, LG_RISING_EDGE, GPIO_PIN, -1) < 0) 
	{
		retVal = GPIO_ALERT_FAIL;
		goto cleanup_claim_fail;
	}

	lguSleep(2);

	printf("Humidity: %d%%RH\nTemp: %d Celsius\n", byteBin2Dec(bits, 0), byteBin2Dec(bits, 16));

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
	bitsReceived++;

	uint64_t latestTimestamp = alerts[num_alerts - 1].report.timestamp;

	// The first two voltage up signals we receive are just the DHT telling us
	// it is ready to send data
	if (bitsReceived <= 2)
	{
		previousTimestamp = latestTimestamp;
		return;
	}
	
	// timestamps are in nanoseconds so we normalize to microseconds
	uint64_t timeDifferenceMicroseconds = (latestTimestamp - previousTimestamp) / 1000;
	// If the time between signals is > 50 it is a 1, otherwise its a 0.
	// Subtract 50 because the DHT pulls voltage down for 50 microseconds between bits
	bits[bitsReceived - 3] = timeDifferenceMicroseconds - 50 < 50 ? 0 : 1;

	previousTimestamp = latestTimestamp;
}

uint8_t byteBin2Dec(int* arr, int n)
{
	int decimalValue = 0;
	int currPower = 7;

	for (int i = n; i < n + 8; i++)
	{
		decimalValue += (int)(exp2(currPower--)) * (*(arr + i));
	}

	return decimalValue;
}
