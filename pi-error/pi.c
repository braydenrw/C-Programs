//Brayden Roth-White
//CS 330
//Estimates of Pi

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

/*
 * f(x) representation
 */
long double f(long double x) {
    return 4.0L/(1.0L+(x*x));
}

/*
 * Trapezoid Rule
 */
long double trapezoid(long double (*f)(long double),
        long double a, long double b, int n) {
    assert(a < b);
    const long double h = (b - a)/n;
    long double sum = 0.0;
    long double x = a + h;
    for (int i = 1; i < n; i++, x += h)
        sum += f(x);
    return h*((f(a) + f(b))/2 + sum);
}

/*
 * Simpsons 1/3 Rule
 */
long double simpsons13(long double (*f)(long double),
        long double a, long double b, int n) {
    assert(n % 2 == 0);
    assert(a < b);
    const long double h = (b - a)/n;
    long double s1 = 0.0;
    long double s2 = 0.0;

    for (int i = 1; i < n; i += 2)
        s1 += f(a + i*h);
    
    for (int i = 2; i < n; i += 2)
        s2 += f(a + i*h);

    return h/3*((f(a) + f(b)) + 4*s1 + 2*s2);
}

/*
 * Simpsons 3/8 Rule
 */
long double simpsons38(long double (*f)(long double), 
        long double a, long double b, int n){
    const long double h = (b - a)/n;
    long double sum=f(a)+f(b);
    long double s1 = 0.0;
    long double s2 = 0.0;
    for(int i=1;i<n;i++){
        if(i%3==0){
            s1+=2*f(a+i*h);
        }
        else{
            s2+=3*f(a+i*h);
        }
    }
    return (sum+s1+s2)*(3*h/8);
}

/*
 * Boole's Rule
 */
long double boole(long double (*f) (long double), 
    long double a, long double b, int n) {
    const long double h = (b - a)/n;
    long double sum=7*(f(a)+f(b));
    long double s1 = 0.0;
    long double s2 = 0.0;
    long double s3 = 0.0;
    for (int i = 4; i < n; i += 4)
        s1 += 14*f(a + i*h);
    for (int i = 1; i < n; i += 2)
        s2 += 32*f(a + i*h);
    for (int i = 2; i < n; i += 4)
        s3 += 12*f(a + i*h);
    return (sum+s1+s2+s3)*(2*h/45);
}

int main(void) {
    const unsigned maxn = 800000;
    for (unsigned n = 12; n <= maxn; n *= 2) {
        const long double I_trap = trapezoid(f, 0, 1, n);
        const long double error_trap = fabsl(M_PI - I_trap);
        const long double I_simp = simpsons13(f, 0, 1, n);
        const long double error_simp = fabsl(M_PI - I_simp);
        const long double I_simp38 = simpsons38(f, 0, 1, n);
        const long double error_simp38 = fabsl(M_PI - I_simp38);
        const long double I_booles = boole(f, 0, 1, n);
        const long double error_booles = fabsl(M_PI - I_booles);
        printf("%5d %0.10Le %0.10Le %0.10Le %0.10Le\n", n,
            error_trap, error_simp ,error_simp38,error_booles);
    }
    return 0;
}
