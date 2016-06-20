#ifndef PORT_CONTROL_H
#define PORT_CONTROL_H
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	#include <errno.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/io.h>

	bool pin_write(int, std::string); // port, link to log file
#endif