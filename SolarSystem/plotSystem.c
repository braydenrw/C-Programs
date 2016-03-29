//Brayden Roth-White
//CS 261
//March 13 2015

#define ARRAY_SIZE 15
#define MAX_VAR_SIZE 256
#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plot.h>
#include <unistd.h>
#include <ctype.h>

#define pi 3.14159
#define PLANETS 5
#define FINALTIME 864000000.0
#define dt 300.0
#define DAY 86400.0
#define GRAVITY 6.67259E-11

typedef struct planetInfo {
	char name[20], color[20];
	double size, mass, xPos, yPos, xVel, yVel;
	double xPos2, yPos2, xVel2, yVel2;
	double year;
	int start;
}planetInfo;

planetInfo array[ARRAY_SIZE];
planetInfo last[ARRAY_SIZE];

/* gets data from a file line by line */
void get_data(char* data) {
	FILE* fd;
	char line[400];
	int i = 0;
	fd = fopen(data, "r");

	/* buffer size 400 gets a line */
	while((fgets(line, 400, fd)) != NULL) {
		if(line[0] == '#' || line[0] == '\n') { //not empty line or commented
			continue;
		}

		sscanf(line, "%s %s %lf %lf %lf %lf %lf %lf", 
			array[i].name, array[i].color, &array[i].size, &array[i].mass,
			&array[i].xPos, &array[i].yPos, &array[i].xVel, &array[i].yVel);

		i++;
	}
	fclose(fd);
}

/* Returns either - or + start position */
void get_start() {
	for(int i = 0; i < ARRAY_SIZE; i++) {
		if(array[i].xPos > 0) { //return 1 if + xPos, -1 otherwise
			array[i].start = 1;
		} else {
			array[i].start = -1;
		}
	}
}

/* Draws 5 planets (should draw 6 but sleep or flushpl is weird) */
void plot(plPlotter* plotter) {
	double scalar = 250000000000;
	for(int i = 0; i <= PLANETS; i++) {
		pl_flushpl_r(plotter);
		pl_pencolorname_r(plotter, "white");
		pl_fillcolorname_r(plotter, array[i].color);
		pl_fcircle_r(plotter, array[i].xPos, array[i].yPos, array[i].size*scalar);
		pl_flushpl_r(plotter);
	}
}

/* draws over previous planets in black */
void deletePlot(plPlotter* plotter) {
	double scalar = 250000000000;
	usleep(50000);
	for(int i = 0; i <= PLANETS; i++) {
		pl_flushpl_r(plotter);
		pl_pencolorname_r(plotter, "black");
		pl_fillcolorname_r(plotter, "black");
		pl_fcircle_r(plotter, array[i].xPos, array[i].yPos, array[i].size*scalar);
		pl_flushpl_r(plotter);
	}
}

/* gets the last planet size and grows the window to fit */
double windowSize(int a) {
	return (fabs(array[a-1].xPos)*1.2);
}

/* set up the plotter and the params */
plPlotter* setUp() {
	plPlotter* plotter;
	plPlotterParams* plotterParams;

	/* create a plotter parametric structure */
	plotterParams = pl_newplparams();
	pl_setplparam(plotterParams, "BITMAPSIZE", "750x750");
	pl_setplparam(plotterParams, "USE_DOUBLE_BUFFERING", "no");
	pl_setplparam(plotterParams, "BG_COLOR", "black");

	/* create the plotter device and open it */
	if((plotter = pl_newpl_r("X", stdin, stdout, stderr, plotterParams)) == NULL) {
		fprintf(stderr, "Couldn't create Xwindows plotter\n");
		exit(1);
	} else if(pl_openpl_r(plotter) < 0) {
		fprintf(stderr, "Couldn't open Xwindows plotter\n");
		exit(1);
	}

	/* set our coordinate space in the plotter window */
	double winSize = windowSize(PLANETS);
	pl_fspace_r(plotter, -winSize, -winSize, winSize, winSize);

	/* pick a type for the pen and the fill */
	pl_pentype_r(plotter, 1);
	pl_filltype_r(plotter, 1);

	return plotter;

}

void cleanUp(plPlotter* plotter) {
	/* close and cleanup the plotter */
	if(pl_closepl_r(plotter) < 0) {
		fprintf(stderr, "Couldn't close plotter \n");
	} else if( pl_deletepl_r(plotter) < 0) {
		fprintf(stderr, "Couldn't delete plotter\n");
	}
}

/* updates xPos, yPos, xVel, and yVel between a time period 'dt' */
void update() {
	for(int i = 0; i < PLANETS; i++) { //Step through each body
		double accelX = 0.0, accelY = 0.0;

		for(int j = 0; j < PLANETS; j++) { //Use the gravity of each other body
			double diffX = array[j].xPos - array[i].xPos;
			double diffY = array[j].yPos - array[i].yPos;
			double r = sqrt(diffX * diffX + diffY * diffY);
			if(i != j) { //Not the same body's gravity
				accelX += (((array[j].xPos - array[i].xPos) * array[j].mass * GRAVITY) / pow(r,3));
				accelY += (((array[j].yPos - array[i].yPos) * array[j].mass * GRAVITY) / pow(r,3));
			}
		}

		array[i].xVel2 = array[i].xVel + (dt * accelX);
		array[i].yVel2 = array[i].yVel + (dt * accelY);

		array[i].xPos2 = array[i].xPos + (dt * array[i].xVel);
		array[i].yPos2 = array[i].yPos + (dt * array[i].yVel);
	}

	for(int i = 0; i < PLANETS; i++) { //Update values for next itteration
		array[i].xVel = array[i].xVel2;
		array[i].yVel = array[i].yVel2;
		array[i].xPos = array[i].xPos2;
		array[i].yPos = array[i].yPos2;
	}
}

/* Tracks when a planet gets back to roughly its start point of the orbit */
void orbitTime(double t) {
	for(int i = 1; i < PLANETS; i++) { //Step through each planet
		if(array[i].start > 0) { //Does it start + or - x direction
			if(array[i].year == 0.0 && array[i].yPos <= 0.0) { //has it made it half way
				array[i].year = 0.5;
			} else if(array[i].year == 0.5 && array[i].yPos >= 0.0) { //has it made it all the way
				array[i].year = 1.0;
			}
		} else if(array[i].start < 0) { // - x side
			if(array[i].year == 0.0 && array[i].yPos >= 0.0) { //half way
				array[i].year = 0.5;
			} else if(array[i].year == 0.5 && array[i].yPos <= 0.0) { //full orbit
				array[i].year = 1.0;
			}
		}
		if(array[i].year == 1.0) { //its just made its first rough orbit
			array[i].year = -1.0;
			double days = (t/DAY);
			printf("%s completes 1 orbit in %lf days.\n", array[i].name, days);
		}
	}
}

/* Plots the orbit, performs update, plot and, deletePlot */
void orbitPlot(plPlotter* plotter) {
	double time = 0.0;
	long long i = 0;

	for(i = 0; i < FINALTIME/dt; i++) { //dt increments until FINALTIME
		update();
		orbitTime(time);
		if(i % 500 == 0) { //only plot after so many updates
			plot(plotter);
			deletePlot(plotter);
		}
		time = time + dt;
	}
	pl_erase_r(plotter);
}

int main(int argc, char** argv) {
	get_data(argv[1]);
	get_start();
	plPlotter* plotter = setUp();

	orbitPlot(plotter);
	
	cleanUp(plotter);
	return 0;
}



