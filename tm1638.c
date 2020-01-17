#include <wiringPi.h>
#include <wiringShift.h>
#include "tm1638.h"

// just like strlen except periods don't count
uint8_t tmstrlen(char s[]) {

	uint8_t result;
	char c, *p;

	result = 0;
	p = s;
	while ((c = *p++) != '\0')
		if (c != '.')
			result++;
	return result;
}

void sendData(uint8_t value) {

	shiftOut(DATA, CLOCK, LSBFIRST, value);
	delay(1);
}

void sendCommand(uint8_t value) {

	digitalWrite(STROBE, LOW);
	sendData(value);
	digitalWrite(STROBE, HIGH);
}

void clearDisplay() {

	int i;

	sendCommand(0x40);		// set auto increment mode
	digitalWrite(STROBE, LOW);
	sendData(0xc0);			// set starting address to 0
	for (i = 0; i < 16; i++)
		sendData(0x00);
	digitalWrite(STROBE, HIGH);
}

uint8_t readButtons(void) {

	uint8_t i, buttons = 0;

	digitalWrite(STROBE, LOW);
	sendData(0x42);

	pinMode(DATA, INPUT);

	for (i = 0; i < 4; i++) {
		uint8_t v = shiftIn(DATA, CLOCK, LSBFIRST) << i;
		delay(1);
		buttons |= v;
	}

	pinMode(DATA, OUTPUT);

	digitalWrite(STROBE, HIGH);
	return buttons;
}

void setLED(uint8_t value, uint8_t position) {

	pinMode(DATA, OUTPUT);

	sendCommand(0x44);

	digitalWrite(STROBE, LOW);
	sendData(0xc1 + (position << 1));
	sendData(value);
	digitalWrite(STROBE, HIGH);
}

void setLEDs(uint8_t value) {

	uint8_t i, mask = 0x80;

	for (i = 0; i < 8; i++) {
		uint8_t mask = 0x1 << i;
		setLED(value & mask ? 1 : 0, 7 - i);
	}
}

void clearLEDs() {

	setLEDs(0);
}

void showString(char *string, char flag) {

	char *p, buffer[128];
	uint8_t i, segments;

	if (flag == 'L') {

		// left justify
		strcpy(buffer, string);
		while (strlen(buffer) < 8)
			strcat(buffer, " ");
	} else {

		// right justify
		buffer[0] = '\0';
		for (i = 0; i < 8 - tmstrlen(string); i++)
			strcat(buffer, " ");
		strcat(buffer, string);
	}

	i = 0;
	for (p = buffer; p != '\0'; p++) {
		segments = 0x00;
		switch (*p) {
			case 'D':
			case '0':
			case 'O':
				segments = 0x3f;
				break;
			case '1':
			case 'I':
				segments = 0x06;
				break;
			case '2':
				segments = 0x5b;
				break;
			case '3':
				segments = 0x4f;
				break;
			case '4':
				segments = 0x66;
				break;
			case '5':
			case 'S':
				segments = 0x6d;
				break;
			case '6':
				segments = 0x7d;
				break;
			case '7':
				segments = 0x07;
				break;
			case '8':
			case 'B':
				segments = 0x7f;
				break;
			case '9':
				segments = 0x6f;
				break;
			case '-':
				segments = 0x40;
				break;
			case 'E':
				segments = 0x79;
				break;
			case 'H':
				segments = 0x76;
				break;
			case 'L':
				segments = 0x38;
				break;
			case '*':
				segments = 0x63;
				break;
			case 'F':
				segments = 0x71;
				break;
			case 'A':
			case 'R':
				segments = 0x77;
				break;
			case 'C':
				segments = 0x39;
				break;
			case 'U':
				segments = 0x3e;
				break;
			case 'P':
				segments = 0x73;
				break;
			case 'r':
				segments = 0x50;
				break;
			case 'o':
				segments = 0x5c;
				break;
			case 'b':
				segments = 0x7c;
				break;
			case 'c':
				segments = 0x58;
				break;
			case 'u':
				segments = 0x1c;
				break;
			case 'd':
				segments = 0x5e;
				break;
			case 'n':
				segments = 0x54;
				break;
			case 'h':
				segments = 0x74;
				break;
		}
		if (*(p + 1) == '.') {
			segments |= 0x80;
			p++;
		}

		digitalWrite(STROBE, LOW);
		sendData(0xc0 + (i << 1));
		sendData(segments);
		digitalWrite(STROBE, HIGH);

		if (i++ >= 7)
			return;
	}
}

void initDisplay() {

	// Initialize display
	wiringPiSetup();
	pinMode(STROBE, OUTPUT);
	pinMode(CLOCK, OUTPUT);
	pinMode(DATA, OUTPUT);
	sendCommand(0x00);  // deactivate
	sendCommand(0x88);  // activate and set brightness 88 to 8f
}
