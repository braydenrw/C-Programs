plotSystem: plotSystem.o
	gcc -g -o plotSystem plotSystem.o -lplot -lXaw -lXmu -lXt -lXext -lSM -lICE -lX11 -lpng -lz -lm

plotSystem.o: plotSystem.c
	gcc -g -Wall -pedantic -std=c99 -c plotSystem.c
