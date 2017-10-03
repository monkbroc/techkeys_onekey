
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

#define BLUE_LED 0
#define KEY1_LED 1
#define KEY1_SWITCH 2

#define DEBOUNCE_TIMER 10

#define LED_ON false
#define LED_OFF true

const char allLeds[] = { BLUE_LED, KEY1_LED };
#define NUM_ALL_LEDS ARR_SZ(allLeds)

const struct keysConfig_t {
	char switchPin;
	char ledPin;
} keysConfig[] = {
	{ KEY1_SWITCH, KEY1_LED },
};
#define NUM_KEYS ARR_SZ(keysConfig)

const uint8_t keyString[] = {
	KS, KU, KD, KO, KSPACE, KS, KU, KENTER
};

struct keys_t {
	bool state;
	bool fallingEdge;
	bool risingEdge;
	char debounceTime;
} keys[NUM_KEYS];

void initLeds() {
	for(unsigned i = 0; i < NUM_ALL_LEDS; i++) {
		IO_set(allLeds[i], LED_OFF);
		IO_config(allLeds[i], OUTPUT);
	}
}

void initKeys() {
	memset(&keys, sizeof(keys), 0);
	for(unsigned i = 0; i < NUM_KEYS; i++) {
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

void sendString(const uint8_t* string, unsigned length) {
	for (unsigned i = 0; i < length; ++i) {
		if (string[i] == KSEMICOLON || string[i] == K2) {
			sendKey(KSHIFT, true);
		}
		sendKey(string[i], true);
		sendKey(string[i], false);
		if (string[i] == KSEMICOLON || string[i] == K2) {
			sendKey(KSHIFT, false);
		}
	}
}

void sendKeys() {
	if (keys[0].fallingEdge) {
		sendString(keyString, ARR_SZ(keyString));
	}
	setLed(1, keys[0].state);
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
	for (unsigned i = 0; i < NUM_KEYS; ++i) {
		IO_set(keysConfig[i].ledPin, IO_get(keysConfig[i].switchPin));
	}
}

void MAIN_handle_sof()
{
}
