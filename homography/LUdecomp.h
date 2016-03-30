
#ifndef LUDECOMP_H
#define LUDECOMP_H

typedef struct {
    int N;
    double **LU;   
    short *mutate;
    short d;       
} LUdecomp;

/*
 * Given NxN matrix A (stored as an array of N row ptrs),
 * returns LU decomposition information.
 * If A is singluar, NULL is returned.
 */
double **createMatrix(int N);

LUdecomp *LUdecompose(int N, double **A);

/*
 * Deallocate decomposition info created by LUdecompose.
 */
void LUdestroy(LUdecomp*);

/*
 * Given LU decomposition info for A,
 * solves linear system Ax = b for x.
 * x and b vectors are assumed to be length N.
 */
void LUsolve(LUdecomp *decomp, const double *b, double *x);

#endif /* LUDECOMP_H */
