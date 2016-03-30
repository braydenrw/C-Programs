#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "LUdecomp.h"

double **createMatrix(int N) {
  double **M = (double **) malloc(N*sizeof(double*));
  for (int i = 0; i < N; i++)
    M[i] = (double*) malloc(N*sizeof(double));
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      M[i][j] = (i == j) ? 1.0 : 0.0;
  return M;
}

void LUdestroy(LUdecomp* M) {
  for (int i = 0; i < M->N; i++)
    free(M->LU[i]);
  free(M->LU);
}

double** swap(int N, int a, int b, double** A){
  double *temp = (double*) malloc(N*sizeof(double));
  for(int i = 0; i < N; i++) {
    temp[i] = A[a][i];
    A[a][i] = A[b][i];
    A[b][i] = temp[i];
  }
  return A;
}

void printMatrix(double** A, int N) {
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      printf("%f, ", A[i][j]);
    }
    printf("\n");
  }
}

LUdecomp *LUdecompose(int N, const double **A) {
  LUdecomp *LU = (LUdecomp*) malloc(sizeof(LUdecomp));
  LU->N = N;
  LU->LU = createMatrix(N);
  LU->mutate = (short *) malloc(N*sizeof(short));
  LU->d = +1;

  // Clone A into LU
  double **A_ = LU->LU;
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      A_[i][j] = A[i][j];

  for (int i = 0; i < N; i++)
    LU->mutate[i] = (short) i;

  // printf("HERE\n");
  // printMatrix(A_, N);
  // printf("END\n\n");

  /*
   * Class notes from 1.1 Partial Pivoting translated c.
   */

  for(int j = 0; j < N; j++) {
    for(int i = 0; i <= j; i++) {
      double sum = 0;
      for(int k = 0; k < i; k++) {
        sum += A_[i][k]*A_[k][j];
      }
      A_[i][j] = A_[i][j] - sum;
    }
    double p = fabs(A_[j][j]);
    int n = j;

    for(int i = j+1; i < N; i++) {
      double sum = 0;
      for(int k = 0; k < j; k++) {
        sum += A_[i][k]*A_[k][j];
      }
      A_[i][j] = A_[i][j] - sum;
      if(fabs(A_[i][j]) > p) {
        p = fabs(A_[i][j]);
        n = i;
      }
    }
    if(p == 0) {
      exit(1);
    }
    if(n != j) {
      // A_ = swap(N, n, j, A_);
      // short temp;
      // temp = LU->mutate[n];
      // LU->mutate[n] = LU->mutate[j];
      // LU->mutate[j] = temp;
      // LU->d = -LU->d;
    }
    for(int i = j+1; i < N; i++) {
      A_[i][j] = A_[i][j]/A_[j][j];
    }
  }

  printf("HERE2\n");
  printMatrix(A_, N);
  printf("END2\n\n");

  return LU;

}


/*
 * Class notes forward substitution and backward substitution 
 * translated in c.
 */
void LUsolve(LUdecomp* decomp, const double *b, double *x) {
    double **B = decomp->LU;
    int N = decomp->N;
    double *y = (double*) malloc(N*sizeof(double));
    y[0] = b[decomp->mutate[0]];

    for(int i = 1; i < N; i++) {
      double sum = 0;
      for(int j = 0; j < i; j++) {
        sum += B[i][j]*y[j];
      }
      y[i] = b[decomp->mutate[i]]-sum;
    }

    x[N-1] = (y[N-1]) / (B[N-1][N-1]);
    for(int i = N-2; i >= 0; i--) {
      double sum = 0;
      for(int j = i+1; j < N; j++) {
        sum += B[i][j]*x[j];
      }
      x[i] = (1/B[i][i]) * (y[i] - sum);
    }
}

