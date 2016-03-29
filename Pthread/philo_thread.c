#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define PHILOSOPHERS 5
#define CHOPSTICKS 5
#define TOTAL_EAT_TIME 100
#define E_MEAN 9
#define T_MEAN 11
#define E_STDEV 3
#define T_STDEV 7
#define MAX_MESSAGE_LEN 256

pthread_t philos[PHILOSOPHERS];
pthread_mutex_t sticks[CHOPSTICKS];
char message[PHILOSOPHERS][MAX_MESSAGE_LEN];

int randomGaussian_r(int mean, int stddev, unsigned int* state) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand_r(state) / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand_r(state) / (double) RAND_MAX;
	if (rand_r(state) & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}

void getRandThink(int *thinkTime, unsigned int *seed) {
	if((*thinkTime = randomGaussian_r(T_MEAN, T_STDEV, seed)) < 0) {
		*thinkTime = 0;
	}
}

void getRandEat(int *eatTime, unsigned int *seed) {
	if((*eatTime = randomGaussian_r(T_MEAN, T_STDEV, seed)) < 0) {
		*eatTime = 0;
	}
}

void think(int *totalTime, int thinkTime, int philoID) {
	int temp = *(int*)totalTime;
	printf("Philo %d will think for %d seconds (%d seconds total)\n",
		philoID, thinkTime, temp);

	*totalTime += thinkTime;
	sleep(thinkTime);
}

void eat(int *totalTimeToEat, int eatTime, int philoID) {
	int temp = *(int*)totalTimeToEat;
	printf("Philo %d will eat for %d seconds (%d seconds total)\n",
		philoID, eatTime, TOTAL_EAT_TIME - temp);

	*totalTimeToEat -= eatTime;
	sleep(eatTime);
}

void *philoCommand(void *num) {
	int philoID = *(int*)num;
	unsigned int seed = (int)time(NULL) + philoID;
	int thinkTime = 0;
	int totalTime = 0; 
	int eatTime = 0;
	int totalTimeToEat = TOTAL_EAT_TIME;
	int leftStick = philoID;
	int rightStick = (philoID + 1) % PHILOSOPHERS;

	while(totalTimeToEat > 0) {
		getRandThink(&thinkTime, &seed);
		think(&totalTime, thinkTime, philoID);
		if(pthread_mutex_trylock(&sticks[leftStick]) != EBUSY) {
			if(pthread_mutex_trylock(&sticks[rightStick]) != EBUSY) {
				getRandEat(&eatTime, &seed);
				eat(&totalTimeToEat, eatTime, philoID);
				pthread_mutex_unlock(&sticks[leftStick]);
				pthread_mutex_unlock(&sticks[rightStick]);
			} else {
				pthread_mutex_unlock(&sticks[leftStick]);
			}
		}
	}

	printf("Philosopher %d has finished eating and left the table.\n", philoID);
	sprintf(
		message[philoID], 
		"Philosopher %d has finished eating for %d and thinking for %d\n",
		philoID, TOTAL_EAT_TIME - totalTimeToEat, totalTime
	);
	return NULL;
}

int main(int argc, char **argv) {
	int params[PHILOSOPHERS][(int)sizeof(int)];

	for(int i = 0; i < CHOPSTICKS; i++) {
		pthread_mutex_init(&sticks[i], NULL);
	}
	for(int i = 0; i < PHILOSOPHERS; i++) {
		*params[i] = i;
		pthread_create(&philos[i], NULL, philoCommand, params[i]);
	}
	for(int i = 0; i < PHILOSOPHERS; i++) {
		pthread_join(philos[i], NULL);
	}
	for(int i = 0; i < PHILOSOPHERS; i++) {
		printf("%s", message[i]);
	}
	return 0;
}