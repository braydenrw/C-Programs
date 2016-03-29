#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <semaphore.h>

#define PHILOSOPHERS 5
#define CHOPSTICKS 5
#define TOTAL_EAT_TIME 100
#define E_MEAN 9
#define T_MEAN 11
#define E_STDEV 3
#define T_STDEV 7

long pids[PHILOSOPHERS];
int semid;

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}

int pickUp(int leftStick, int rightStick) {
	struct sembuf lock[2];
	lock[0].sem_num = leftStick;
	lock[0].sem_op = -1;
	lock[0].sem_flg = 0;
	lock[1].sem_num = rightStick;
	lock[1].sem_op = -1;
	lock[1].sem_flg = 0;
	return semop(semid, lock, 2);
}

int putDown(int leftStick, int rightStick) {
	struct sembuf unlock[2];
	unlock[0].sem_num = leftStick;
	unlock[0].sem_op = 1;
	unlock[0].sem_flg = 0;
	unlock[1].sem_num = rightStick;
	unlock[1].sem_op = 1;
	unlock[1].sem_flg = 0;
	return semop(semid, unlock, 2);
}

void philoCommand(int num) {
	int leftStick, rightStick;
	int thinkTime = 0;
	int totalTime = 0; 
	int eatTime = 0;
	int totalTimeToEat = TOTAL_EAT_TIME;
	leftStick = num;
	totalTime = 0;

	srand((int)time(NULL) + num);

	if(num == PHILOSOPHERS - 1) {
		rightStick = 0;
	} else {
		rightStick = num + 1;
	}

	while(totalTimeToEat > 0) {
		if((thinkTime = randomGaussian(T_MEAN, T_STDEV)) < 0) {
			thinkTime = 0;
		}

		printf("Philo %d will think for %d seconds (%d seconds total)\n",
			num, thinkTime, totalTime);

		totalTime += thinkTime;
		sleep(thinkTime);

		if((eatTime = randomGaussian(T_MEAN, T_STDEV)) < 0) {
			eatTime = 0;
		}

		if((pickUp(leftStick, rightStick) == -1) && (errno == EINTR)) {
			fprintf(stderr, "Error with semop locking: %s\n", strerror(errno));
			exit(1);
		}

		printf("Philo %d will eat for %d seconds (%d seconds total)\n",
			num, eatTime, TOTAL_EAT_TIME - totalTimeToEat);

		totalTimeToEat -= eatTime;

		sleep(eatTime);
		if((putDown(leftStick, rightStick) == -1) && (errno == EINTR)) {
			fprintf(stderr, "Error with semop unlocking: %s\n", strerror(errno));
			exit(1);
		}
	}
	printf("Philosopher %d has finished eating for %d and thinking for %d ",
		num, TOTAL_EAT_TIME - totalTimeToEat, totalTime);

	exit(0);
}

int main(int argc, char **argv) {
	semid = semget(IPC_PRIVATE, CHOPSTICKS, S_IRUSR | S_IWUSR);

	for(int i = 0; i < PHILOSOPHERS; i++){
		struct sembuf init;
		init.sem_num = i;
		init.sem_op = 1;
		init.sem_flg = 0;
		semop(semid, &init, 1);
	}

	for(int i = 0; i < PHILOSOPHERS; i++) {
		if((pids[i] = fork()) < 0) {
			fprintf(stderr,"Error with fork: %s\n", strerror(errno));
			abort();
		} else if(!pids[i]) {
			philoCommand(i);
		} 
	}

	int status;
	long pid;
	int remainingPhilos = PHILOSOPHERS;
	while(remainingPhilos) {
		pid = wait(&status);
		printf("(PID %ld exited with code %x.)\n", pid, status);
		remainingPhilos--;
	}

	return 0;
}

