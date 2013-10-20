/*  libkorad.c
 *  Copyright (C) 2013 Tomasz Głuch
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <strings.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>

#include "libkorad.h"

int fd;
struct termios oldtio, newtio;
char buf[255];
struct koradstatus_t status;

/* private functions */
int korad_query(const char *query, char *replybuffer, size_t responselenght);
double korad_get_double(const char *query, size_t responselenght);

void debug_tio(struct termios *tcio) {
	printf("TCIO iflag: \n ");
	if (tcio->c_iflag & IGNBRK) {
		printf("IGNBRK ");
	}
	if (tcio->c_iflag & BRKINT) {
		printf("BRKINT ");
	}
	if (tcio->c_iflag & IGNPAR) {
		printf("IGNPAR ");
	}
	if (tcio->c_iflag & PARMRK) {
		printf("PARMRK ");
	}
	if (tcio->c_iflag & INPCK) {
		printf("INPCK ");
	}
	if (tcio->c_iflag & ISTRIP) {
		printf("ISTRIP ");
	}
	if (tcio->c_iflag & INLCR) {
		printf("INLCR ");
	}
	if (tcio->c_iflag & IGNCR) {
		printf("IGNCR ");
	}
	if (tcio->c_iflag & ICRNL) {
		printf("ICRNL ");
	}
	if (tcio->c_iflag & IUCLC) {
		printf("IUCLC ");
	}
	if (tcio->c_iflag & IXON) {
		printf("IXON ");
	}
	if (tcio->c_iflag & IXANY) {
		printf("IXANY ");
	}
	if (tcio->c_iflag & IXOFF) {
		printf("IXOFF ");
	}
	printf("\nTCIO oflag: \n ");
	if (tcio->c_oflag & OPOST) {
		printf("OPOST ");
	}
	if (tcio->c_oflag & ONLCR) {
		printf("ONLCR ");
	}
	if (tcio->c_oflag & OCRNL) {
		printf("OCRNL ");
	}
	if (tcio->c_oflag & ONOCR) {
		printf("ONOCR ");
	}
	if (tcio->c_oflag & ONLRET) {
		printf("ONLRET ");
	}
	if (tcio->c_oflag & OFILL) {
		printf("OFILL ");
	}
	printf("\nTCIO cflag: \n ");
	if (tcio->c_cflag & CS8) {
		printf("CS8 ");
	}
	if (tcio->c_cflag & CSTOPB) {
		printf("CSTOPB ");
	}
	if (tcio->c_cflag & CREAD) {
		printf("CREAD ");
	}
	if (tcio->c_cflag & PARENB) {
		printf("PARENB ");
	}
	if (tcio->c_cflag & PARODD) {
		printf("PARODD ");
	}
	if (tcio->c_cflag & CLOCAL) {
		printf("CLOCAL ");
	}
	if (tcio->c_cflag & HUPCL) {
		printf("HUPCL ");
	}
	if (tcio->c_cflag & CRTSCTS) {
		printf("CRTSCTS ");
	}
	printf("\nTCIO lflag: \n ");
	if (tcio->c_lflag & ICANON) {
		printf("ICANON ");
	}
	if (tcio->c_lflag & ISIG) {
		printf("ISIG");
	}

}

/*! Initialize serial port
 *
 * @param port Path to serial port device file
 * @return 0 on success, -1 on error
 */

int korad_init(char *port) {
	printf("Using serial port %s\n", port);
	if (0 > (fd = open(port, O_RDWR | O_NOCTTY))) {
		perror(port);
		return (-1);
	}

	/*struct flock lock =  { F_WRLCK, SEEK_SET, 0, 0, 0 };
	 lock.l_pid = getpid();
	 if (fcntl(fd, F_SETLK, &lock )) {
	 perror("fcntl");
	 exit(2);
	 }
	 */

	tcgetattr(fd, &oldtio); /* save current port settings */
	//debug_tio(&oldtio); //exit(2);

	bzero(&newtio, sizeof(newtio));
	/* speed */
	speed_t baud = B9600;
	cfsetospeed(&newtio, baud);
	cfsetispeed(&newtio, baud);

	/* 8N1 */
	newtio.c_cflag |= CS8;

	/* hardware handshake */
	newtio.c_cflag |= CRTSCTS;

	/* other control */
	newtio.c_cflag |= CREAD | CLOCAL;

	//newtio.c_iflag |= IGNBRK;
	//newtio.c_iflag |= IGNPAR;
	newtio.c_iflag |= IXON | IXOFF;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = libkorad_reply_timeout_ms / 100; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 0; /* blocking read until 5 chars received */

	tcflush(fd, TCIOFLUSH);
	if (tcsetattr(fd, TCSANOW, &newtio)) {
		perror("tcsetattr");
	}
	tcflush(fd, TCIOFLUSH);

	memset(buf, 0, sizeof(buf));
	korad_query("*IDN?", (char*) &buf, 32);
	printf("Opened: %s\n", buf);
	return 0;
}

/*!
 * Function closes serial port and restore old settings
 */
int korad_close(void) {
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
	return 1;
}

/*! Low-level query to power supply
 *
 * @param query Pointer to string with query string
 * @param replybuffer Pointer to area where response will be saved, can be NULL when response is not expected
 * @param responselenght Number of bytes expected to be received
 * @return Number of bytes actually received
 */
int korad_query(const char *query, char *replybuffer, size_t responselenght) {
	int bytes;
#ifdef DEBUG
	printf("%s: ", query);
#endif
	tcflush(fd, TCIFLUSH);
	write(fd, query, strlen(query));
	usleep(100000);
	if (responselenght > 0) {
		bytes = read(fd, replybuffer, responselenght);
#ifdef DEBUG
		printf("%d bytes read\n", bytes);
		if (bytes > 0) {printf("%s\n", replybuffer);}
#endif
		return bytes;
		//perror ("read error:");
	}
	return 0;
}

/*! Reads present value of current
 *  @return current or NaN when read failed
 */
double korad_get_current(void) {
	return korad_get_double("IOUT1?", 5);
}

/*! Reads present value of voltage
 *  @return voltage or NaN when read failed
 */
double korad_get_voltage(void) {
	return korad_get_double("VOUT1?", 5);
}

/*! Reads set value of voltage
 *  @return voltage or NaN when read failed
 */
double korad_get_vset(void) {
	return korad_get_double("VSET1?", 5);
}

/*! Reads set value of current
 *  @return current or NaN when read failed
 */
double korad_get_iset(void) {
	return korad_get_double("ISET1?", 5);
}

/*! Fetch status register from device
 *  Should be called before any korad_status_* call
 */
void korad_status_refresh(void) {
	korad_query("STATUS?", (char *) &status, 1);
#ifdef DEBUG
	printf("Status: 0x%x", *((char*)&status));
#endif
}

/*! Get running mode
 * @return Supply status
 */
unsigned int korad_status_mode() {
	return status.ch1_mode;
}

/*! Get output status
 * @return Output status
 */
unsigned int korad_status_output() {
	return status.output;
}

/*! Check if beep is enabled
 * @return Beep status
 */
unsigned int korad_status_beep() {
	return status.beep;
}

/*! Check if overcurrent protection is enabled
 * @return Overcurrent protection status
 */
unsigned int korad_status_ocp() {
	return status.ocp;
}

/*! Check if overvoltage protection is enabled
 * @return Overvoltage protection status
 */
unsigned int korad_status_ovp() {
	return status.ovp;
}

/*! Check if chassis buttons are locked
 * @return Lock status, or false when not supported
 * Note: this function can be used only in older version.
 * Implented according to documentation, but do not appears in newer firmware.
 */
unsigned int korad_status_lock() {
#ifdef FIRMWARE_13
	return status.lock;
#else
	return 0;
#endif
}

/*! Internal function. Send query and returns response converted to double value
 * @param query Query string to send
 * @param responselenght Expected lenght of reply
 * @returns Double value of NaN when read failed
 */
double korad_get_double(const char *query, size_t responselenght) {
	char *buf[16];
	int len;
	memset(buf, 0, sizeof(buf));
	len = korad_query(query, (char*) &buf, responselenght);
	if (len != responselenght) {
		fprintf(stderr, "Unexpected response of %d bytes at %s: %s", len, query,
				(char*) &buf);
		return NAN;
	}
	double current;
	current = atof((char *) buf);
#ifdef DEBUG
	printf("Response: %d bytes: %2.3f\n", len , current);
#endif
	return current;
}

/*! Set output mode
 * @param enabled 1 to enable, 0 to disable
 */
void korad_set_output(unsigned int enabled) {
	if (enabled)
		korad_query("OUT1", NULL, 0);
	else
		korad_query("OUT0", NULL, 0);
}

/*! Set beep mode
 * @param enabled 1 to enable, 0 to disable
 */
void korad_set_beep(unsigned int enabled) {
	if (enabled)
		korad_query("BEEP1", NULL, 0);
	else
		korad_query("BEEP0", NULL, 0);
}

/*! Set overvoltage protection
 * @param enabled 1 to enable, 0 to disable
 */
void korad_set_ovp(unsigned int enabled) {
	if (enabled)
		korad_query("OVP1", NULL, 0);
	else
		korad_query("OVP0", NULL, 0);
}

/*! Set overcurrent protection
 * @param enabled 1 to enable, 0 to disable
 */
void korad_set_ocp(unsigned int enabled) {
	if (enabled)
		korad_query("OCP1", NULL, 0);
	else
		korad_query("OCP0", NULL, 0);
}

/*! Change set value of current
 * @param current Current value as a double
 */
void korad_set_iset(double current) {
	char str[12];
	if (current < 0)
		return;
	if (current > IMAX)
		return;
	snprintf(str, sizeof(str), "ISET1:%1.3f", current);
	korad_query(str, NULL, 0);
}

/*! Change set value of voltage
 * @param voltage Voltage value as a double
 */
void korad_set_vset(double voltage) {
	char str[12];
	if (voltage < 0)
		return;
	if (voltage > VMAX)
		return;
	snprintf(str, sizeof(str), "VSET1:%2.2f", voltage);
	korad_query(str, NULL, 0);
}

/*! Recall settings from selected memory bank
 * @param memory Memory bank to use
 */
void korad_memory_recall(int memory) {
	char str[12];
	snprintf(str, sizeof(str), "RCL%d", memory);
	korad_query(str, NULL, 0);
}

/*! Store settings to selected memory bank
 * @param memory Memory bank to use
 */
void korad_memory_save(int memory) {
	char str[12];
	snprintf(str, sizeof(str), "SAV%d", memory);
	korad_query(str, NULL, 0);
}
