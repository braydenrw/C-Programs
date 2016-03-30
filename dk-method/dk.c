//Brayden Roth-White
//Numerical Computations
//CS 330

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <float.h>

#define MAXN 25 // max N

void kerner() {
  int n = 0;
  complex double c[MAXN+1];
  double a, b;
  while (scanf("%lf %lf", &a, &b) == 2) {
    c[n++] = a + b*I;
  }
  c[n] = 1;
 
  // max coefficient
  double max = 0;
  for (int i = 0; i < n; i++) {
    double v = cabs(c[i]);
    if(v > max) {
      max = v;
    }
  }

  // radius here
  double R = 1 + max;
  double Dtheta = (2*M_PI)/n;
  double theta = 0;
  complex double z[n];
  for(int j = 0; j<n; j++) {
    z[j] = (cos(theta)+ I*sin(theta))*R;
    theta += Dtheta;
  }

  // print a guess
  printf("iter 1 \n");                       
  for (int j = 0; j < n; j++) { 
    printf("z[%i] = %0.10f + %0.10f i\n", j, creal(z[j]), cimag(z[j]));
  }
  complex double dz[n];                  
  for (int k = 1; k < 50; k++) { // iterate through iterations
    complex double zmax = 0;                
    for(int j = 0; j < n; j++) { // compute f(x)
      complex double r =  1;           
      for (int i = n-1; i >= 0; i--) { // horner's method break up poly
        r = c[i] + r * z[j];      
      }
      complex double q = 1;
      for (int i = 0; i < n; i++) { // Helps update the equation
        if (i != j) { 
          q = q * (z[j]- z[i]);
        }
      }
      dz[j] = -(r / q);
      if(cabs(dz[j]) > cabs(zmax)) { // check if delta z is greater
        zmax = cabs(dz[j]);
      }
    }
    printf("iter %i\n", k+1);               
    for (int j = 0; j < n; j++) { // print the rest
      z[j] = z[j] + dz[j];
      printf("z[%i] = %0.10f + %0.10f i\n", j, creal(z[j]), cimag(z[j]));
    }
        
    if(cabs(zmax) <= FLT_EPSILON) { // done here
      break;    
    }
  }
}

int main(int argc, char** argv) {
	kerner();
  return 0;
}