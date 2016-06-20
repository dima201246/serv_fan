#include "../lib/conf_lang.h"
#include "port_control.h"

#define LPT_PORT 0x378

#define CHANNEL_1 0x1
#define CHANNEL_2 0x2
#define CHANNEL_3 0x4
#define CHANNEL_4 0x8
#define CHANNEL_5 0x10
#define CHANNEL_6 0x20
#define CHANNEL_7 0x40
#define CHANNEL_8 0x80
#define ALL_CHANNELS_ON 0xff
#define ALL_CHANNELS_OFF 0x0

bool init_port(unsigned long port) {
	if (ioperm(port, 1, 1) != 0)
		return false;
	return true;
}

unsigned char channel_to_signal(int ch) {
	switch (ch) {
		case 1 : return CHANNEL_1;
		case 2 : return CHANNEL_2;
		case 3 : return CHANNEL_3;
		case 4 : return CHANNEL_4;
		case 5 : return CHANNEL_5;
		case 6 : return CHANNEL_6;
		case 7 : return CHANNEL_7;
		case 8 : return CHANNEL_8;
		case 9 : return ALL_CHANNELS_ON;
		case 0 : return ALL_CHANNELS_OFF;
	}

	return ALL_CHANNELS_OFF;
}

unsigned char pin_status(unsigned long port) {
    return inb(port);
}

void send_signal(unsigned char signal, unsigned long port) {
	outb(signal, port);
}

bool close_port(unsigned long port) {
	if (ioperm(port, 1, 0) != 0)
		return false;
	return true;
}

bool pin_write(int pin, std::string LOG_FILE) {
	int signal = channel_to_signal(pin); // Delaem signal iz kanala

    if (!init_port(LPT_PORT)) {
    	add_to_file(LOG_FILE, "[PORT_CONTROL] ERROR! Cannot open " + str(pin) + " pin, " + (std::string)strerror(errno));
    	return false;
    }

    if ( (signal != ALL_CHANNELS_OFF) && (signal != ALL_CHANNELS_ON) )
        signal ^= pin_status(LPT_PORT); // Dobavlyaem k signalu starye znacheniya

    send_signal(signal, LPT_PORT); // Otpravlyaem signal

    if (!close_port(LPT_PORT)) {
    	add_to_file(LOG_FILE, "[PORT_CONTROL] ERROR! Cannot close " + str(pin) + " pin, " + (std::string)strerror(errno));
    	return false;
    }

    return true;
}