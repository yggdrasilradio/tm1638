#include <stdio.h>
#include <wiringPi.h>
#include <wiringShift.h>
#include <unistd.h>
#include <signal.h>

#include "tm1638.h"

#define PIDFILE "/var/run/tm1638.pid"
#define COMMAND "/tmp/.tm1638command"

char command[128];

void randomNumbers() {

	uint8_t count, buffer[16];
	uint32_t value;

	initDisplay();
	count = 0;
	while (TRUE) {
		setLEDs(count++);
		value = (rand() % (99999999 + 1 - 10000000) + 10000000);
		sprintf(buffer, "%8d", value);
		showString(buffer, 'R');
		delay(250);
	}
}

int fileExists(char *filename) {

	if (access(filename, F_OK) == -1)
		return FALSE;
	else
		return TRUE;
}

writeCommand() {

	FILE *fp = fopen(COMMAND, "w");
	fprintf(fp, "%s\n", command);
	fclose(fp);
}

void stopService() {

	pid_t mypid;

	FILE *fp = fopen(PIDFILE, "r");
	if (fp != NULL) {
		fscanf(fp, "%d\n", &mypid);
		printf("Stopped service, PID %d\n", mypid);
		fclose(fp);
		unlink(PIDFILE);
		kill(mypid, SIGUSR1);
		usleep(70000);
		usleep(70000);
		usleep(70000);
	}
}

void rootCheck() {

	if (access("/dev/mem", R_OK) == -1) {
		printf("You need to be root to perform this command.\n");
		exit(1);
	}
}

void startService(void (*procedure)(void)) {

	pid_t mypid;

	rootCheck();
	stopService();
	mypid = fork();
	if (mypid == 0) {
		// background process
		writeCommand();
		procedure();
	} else {
		// foreground process
		FILE *fp = fopen(PIDFILE, "w");
		fprintf(fp, "%d\n", mypid);
		fclose(fp);
		printf("Started service, PID %d\n", mypid);
	}
	exit(0);
}

int main(int argc, char *argv[]) {

	srand(time(NULL));

	if (argc >= 2) {

		// DISPLAY OFF
		if (strcmp(argv[1], "off") == 0) {
			stopService();
			initDisplay();
			clearDisplay();
			exit(0);
		}

		// DISPLAY STATUS
		if (strcmp(argv[1], "status") == 0) {
			if (fileExists(PIDFILE))
				printf("Display is on.\n");
			else
				printf("Display is off.\n");
			exit(0);
		}

		// DISPLAY ON
		strncpy(command, argv[1], sizeof(command));
		if (strcmp(argv[1], "on") == 0) {
			FILE *fp = fopen(COMMAND, "r");
			if (fp != NULL) {
				fscanf(fp, "%s\n", command);
				fclose(fp);
			} else
				strcpy(command, "demo"); // default animation
		}

		// DISPLAY LEFT JUSTIFIED
		if (strcmp(command, "-l") == 0) {
			stopService();
			initDisplay();
			clearLEDs();
			showString(argv[2], 'L');
			exit(0);
		}

		// DISPLAY RIGHT JUSTIFIED
		if (strcmp(command, "-r") == 0) {
			stopService();
			initDisplay();
			clearLEDs();
			showString(argv[2], 'R');
			exit(0);
		}

		// DISPLAY LEDS
		if (strcmp(command, "lights") == 0) {
			stopService();
			initDisplay();
			setLEDs(atoi(argv[2]));
			exit(0);
		}

		// DISPLAY RANDOM NUMBERS
		if (strcmp(command, "demo") == 0) {
			startService(randomNumbers);
		}
	}

	// USAGE
	printf("Usage:");
	printf("\tdisplay off\t\t\tClear/stop display\n");
	printf("\tdisplay on\t\t\tRestore previous display\n");
	printf("\tdisplay demo\t\t\tDisplay random number/lights\n");
	printf("\tdisplay -l \"HI ALICE\"\t\tDisplay left justified\n");
	printf("\tdisplay -r \"3.141589\"\t\tDisplay right justified\n");
	printf("\tdisplay lights 255\t\tDisplay number (0-255) in lights\n");
	exit(0);
}
