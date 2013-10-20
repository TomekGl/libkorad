/*  libkorad.h
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

#ifndef LIBKORAD_H_
#define LIBKORAD_H_

/** Maximum time to wait for response */
#define libkorad_reply_timeout_ms 500

/** Structure with bit fields to describe current status of device */
#ifdef FIRMWARE_13 /* based on documentation */
struct koradstatus_t {
	unsigned int ch1_mode :1;
	unsigned int ch2_mode :1;
	unsigned int tracking :2;
	unsigned int beep :1;
	unsigned int lock :1;
	unsigned int output :1;
	unsigned int :1; //empty
};
#else /* KORADKA3005PV2.0 */
struct koradstatus_t {
	unsigned int ch1_mode :1;
	unsigned int ch2_mode :1;
	unsigned int tracking :2;
	unsigned int beep :1;
	unsigned int ocp :1;
	unsigned int output :1;
	unsigned int ovp :1;
};
#endif

/** Maximum value of voltage */
#define VMAX 30.1
/** Maximum value of current */
#define IMAX 5.0

#define KORAD_MODE_CC 0
#define KORAD_MODE_CV 1

#define KORAD_TRACKING_INDEPENDENT 0
#define KORAD_TRACKING_SERIES 1
#define KORAD_TRACKING_PARALLEL 3

#define KORAD_UNLOCKED 1
#define KORAD_BEEPENABLED 1
#define KORAD_OUTPUT_ON 1

int korad_init(char *port);
int korad_close(void);

/* gets */
double korad_get_current(void);
double korad_get_voltage(void);
double korad_get_vset(void);
double korad_get_iset(void);

void korad_status_refresh(void);
unsigned int korad_status_mode(void);
unsigned int korad_status_output(void);
unsigned int korad_status_beep(void);
unsigned int korad_status_lock(void);
unsigned int korad_status_ocp(void);
unsigned int korad_status_ovp(void);

/* sets */
void korad_set_output(unsigned int);
void korad_set_beep(unsigned int);
void korad_set_ocp(unsigned int);
void korad_set_ovp(unsigned int);

void korad_set_iset(double current);
void korad_set_vset(double voltage);

void korad_memory_recall(int memory);
void korad_memory_save(int memory);
#endif /* LIBKORAD_H_ */
