//Brayden Roth-White
//Homography
//11388543

#include "LUdecomp.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

void decompAndSolveMatrix(double **A, double *B) {
    double X[8];
    double **H = createMatrix(3);
    int z = 0;
    LUdecomp *LU = LUdecompose(8, A);
    LUsolve(LU, B, X);
    
    for(int i = 0; i < 3; i++) { 
        for(int j = 0; j < 3; j++) {
            if(i == 2 && j == 2) {
                H[i][j] = 1;
            } else {
                H[i][j] = X[z];
                z++;
            }
        }
    }
    printMatrix(H);
}

void printMatrix(double **H) {
    for(int i = 0; i < 3; i++) { 
        for(int j = 0; j < 3; j++) {
            printf("%lf ", H[i][j]);
        }
        printf("\n");
    }
}

void equation7(double *xIn, double *yIn, 
    double *xOut, double *yOut, double **A, 
    double*B, int n) {

    for(int i = 0; i < n; i++) {
        scanf("%lf %lf", &xIn[i], &yIn[i]);
    }
    for(int i = 0; i < n; i++) {
        scanf("%lf %lf", &xOut[i], &yOut[i]);
    }
    
    for(int i = 0, j = 0; i < n*2; i+=2, j++) { 
        int k = i+1;
        A[i][0] = xIn[j];
        A[i][1] = yIn[j];
        A[i][2] = 1;
        A[i][3] = 0;
        A[i][4] = 0;
        A[i][5] = 0;
        A[i][6] = -1 * xIn[j] * xOut[j];
        A[i][7] = -1 * yIn[j] * xOut[j];
        B[i] = xOut[j];
        
        A[k][0] = 0;
        A[k][1] = 0;
        A[k][2] = 0;
        A[k][3] = xIn[j];
        A[k][4] = yIn[j];
        A[k][5] = 1;
        A[k][6] = -1 * xIn[j] * yOut[j];
        A[k][7] = -1 * yIn[j] * yOut[j];
        B[k] = yOut[j];
    }

}

int main(){
    int n;
    scanf("%d", &n);
    double xIn[n], yIn[n];
    double xOut[n], yOut[n];
    double **A = createMatrix(n*2);
    double B[8];
    equation7(xIn, yIn, xOut, yOut, A, B, n);
    decompAndSolveMatrix(A, B);
    return 0;
}
