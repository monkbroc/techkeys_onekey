#include "platforms.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "usb_keyboard.h"
#include "io.h"
#include "hid.h"
#include "timer.h"
#include "auxiliary.h"

#define NUM_KEYS 3

#define BLUE_LED 0
#define KEY1_LED 1
#define KEY2_LED 2
#define KEY3_LED 3
#define KEY1_SWITCH 4
#define KEY2_SWITCH 5
#define KEY3_SWITCH 6

#define DEBOUNCE_TIMER 10

#define LED_ON false
#define LED_OFF true

const char allLeds[] = { BLUE_LED, KEY1_LED, KEY2_LED, KEY3_LED };

const struct keysConfig_t {
	char switchPin;
	char ledPin;
	int keycode;
} keysConfig[NUM_KEYS] = {
	{ KEY1_SWITCH, KEY1_LED, KCTRL},
	{ KEY2_SWITCH, KEY2_LED, KALT},
	{ KEY3_SWITCH, KEY3_LED, KDELETE},
};

struct keys_t {
	bool state;
	bool fallingEdge;
	bool risingEdge;
	char debounceTime;
} keys[NUM_KEYS];

void initLeds() {
	for(unsigned char i = 0; i < sizeof(allLeds)/sizeof(allLeds[0]); i++) {
		IO_set(i, LED_OFF);
		IO_config(i, OUTPUT);
	}
}

void initKeys() {
	memset(&keys, sizeof(keys), 0);
	for(unsigned char i = 0; i < NUM_KEYS; i++) {
		IO_config(keysConfig[i].switchPin, INPUT);
		IO_set(keysConfig[i].switchPin, true);
	}
}

void debounce() {
	for(unsigned char i = 0; i < NUM_KEYS; i++) {
		char newState;
		keys[i].fallingEdge = false;
		keys[i].risingEdge = false;

		newState = IO_get(keysConfig[i].switchPin);
		if(keys[i].debounceTime > 0) {
			keys[i].debounceTime--;
		} else if(!keys[i].state && newState) {
			keys[i].risingEdge = true;
			keys[i].state = true;
			keys[i].debounceTime = DEBOUNCE_TIMER;
		} else if(keys[i].state && !newState) {
			keys[i].fallingEdge = true;
			keys[i].state = false;
			keys[i].debounceTime = DEBOUNCE_TIMER;
		}
	}
}

void sendKey(int code, bool state) {
	HID_set_scancode_state(code, state);
	HID_commit_state();
	_delay_ms(3);
}

void setLed(int pin, bool state) {
	IO_set(pin, state);
}

void sendKeys() {
	for(unsigned char i = 0; i < NUM_KEYS; i++) {
		if(keys[i].fallingEdge) {
			sendKey(keysConfig[i].keycode, true);
			setLed(keysConfig[i].ledPin, LED_ON);
		} else if(keys[i].risingEdge) {
			sendKey(keysConfig[i].keycode, false);
			setLed(keysConfig[i].ledPin, LED_OFF);
		}
	}
}

void setup() {
	clock_prescale_set(clock_div_1);

	USB_init();
	HID_commit_state();

	initLeds();
	initKeys();

	TIMER_init();
}

void loop() {
	debounce();

	sendKeys();
	_delay_ms(1);
}

int main(void)
{
	setup();

	for(;;) {
		loop();
	}

}

void MAIN_timer_handler()
{
	//for (int i = 0; i < 3; ++i)
	//	IO_set(1+i, IO_get(4+i));
}

void MAIN_handle_sof()
{
}
