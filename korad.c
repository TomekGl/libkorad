/*  korad.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "libkorad.h"

void print_status() {
	korad_status_refresh();
	printf("CV/CC: %s\tBEEP: %d\t OUT: %d\t LOCK: %d\tOCP: %d\tOVP: %d\n",
			(korad_status_mode() == KORAD_MODE_CV) ? "CV" : "CC",
			korad_status_beep(), korad_status_output(), korad_status_lock(),
			korad_status_ocp(), korad_status_ovp());

}

void sigquit_handler(int signum) {
	korad_close();
	printf("\nBye!\n");
	exit(0);
}

int main() {
	struct sigaction act;
	memset(&act, 0, sizeof(act));

	act.sa_handler = sigquit_handler;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("sigaction");
		exit(-1);
	}
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror("sigaction");
		exit(-1);
	}

	if (korad_init("/dev/ttyACM0") < 0) {
		printf("Failed to initialize port\n");
		return 1;
	}

	korad_set_ovp(0);
	korad_set_ocp(0);

	korad_set_output(1);
	print_status();
	korad_memory_recall(3);
	korad_set_ovp(0);
	usleep(100000);
	print_status();
//  korad_set_beep(1);
//  usleep(100000);
//  print_status();
	korad_set_ocp(0);
	usleep(100000);
	print_status();

	double i, u;
//i = 0.0;
//u = 0.0;
	i = korad_get_iset();
	u = korad_get_vset();
	while (1) {
		double tmp;
		tmp = korad_get_current();
		printf("Curr: %1.3fA\t", tmp);
		tmp = korad_get_iset();
		printf("ISET: %1.3fA\t", tmp);
		tmp = korad_get_voltage();
		printf("V: %2.3fV\t", tmp);
		tmp = korad_get_vset();
		printf("VSET: %2.3fV\n", tmp);
		//usleep(20000);
		print_status();
		i += 0.008;
		u = 12;
		korad_set_vset(u);
		korad_set_iset(i);

	}

	korad_close();
	return 0;
}
