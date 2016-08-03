// Copyright (c) 2014-2016, Massachusetts Institute of Technology
//
// This file is part of the Compressed Continuous Computation (C3) toolbox
// Author: Alex A. Gorodetsky 
// Contact: goroda@mit.edu

// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, 
//    this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice, 
//    this list of conditions and the following disclaimer in the documentation 
//    and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its contributors 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//Code

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <complex.h>

#define BASESIZE 2
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/***********************************************************//**
    Compute DFT
***************************************************************/
int fft_slow(size_t N, const double complex * xin, size_t sx, 
             double complex * xout, size_t sX)
{
    for (size_t ii = 0; ii < N; ii++){
        xout[ii*sX] = 0.0;
        for (size_t jj = 0; jj < N; jj++){
            xout[ii*sX] += cexp(- 2 * M_PI * I * ii * jj / N) * xin[jj*sx];
        }
    }
    return 0;
}

/***********************************************************//**
    Compute IDFT
***************************************************************/
int ifft_slow(size_t N, const double complex * xin, size_t sx, 
              double complex * xout, size_t sX)
{
    for (size_t ii = 0; ii < N; ii++){
        xout[ii*sX] = 0.0;
        for (size_t jj = 0; jj < N; jj++){
            xout[ii*sX] += cexp( 2 * M_PI * I * ii * jj / N) * xin[jj*sx];
        }
        xout[ii*sX] /= (double)N;
    }
    return 0;
}

/***********************************************************//**
   FFT                                                            
***************************************************************/
int fft_base(size_t N, const double complex * x, size_t sx, 
             double complex * X, size_t sX)
{
    // assumes N is a power of 2
    if (N < BASESIZE){
        int res = fft_slow(N,x,sx,X,sX);
        return res;
    }
    else{
        fft_base(N/2,x,2*sx,X,2*sX);
        fft_base(N/2,x+sx,2*sx,X+sX,2*sX);

        complex double * e = malloc(N/2 * sizeof(complex double));
        complex double * o = malloc(N/2 * sizeof(complex double));

        for (size_t ii = 0; ii < N/2; ii++){
            e[ii] = X[ii*2*sX];
            o[ii] = X[ii*2*sX+sX];
        }

        for (size_t ii = 0; ii < N/2; ii ++){
            double complex twiddle = cexp(- 2 * M_PI * I * ii  / N);
            /* printf("ii=%zu, twiddle = (%G,%G)\n",ii,creal(twiddle),cimag(twiddle)); */
            
            /* printf("\t ind = %zu, %zu\n",ii*2*sX, ii*2*sX+sX); */
            /* printf("\te = (%G,%G), o = (%G,%G)\n", creal(e),cimag(e), creal(o),cimag(o)); */
            X[ii*sX] = e[ii] + twiddle * o[ii];
            X[ii*sX + N/2*sX] = e[ii] - twiddle * o[ii];
        }
        free(e); e = NULL;
        free(o); o = NULL;

        return 0;
    }
    
}

/***********************************************************//**
   IFFT                                                            
***************************************************************/
int ifft_base(size_t N, double complex * x, size_t sx, 
             double complex * X, size_t sX)
{
    for (size_t ii = 0; ii < N; ii++){
        x[ii] = conj(x[ii*sX]);
    }
    int res = fft_base(N,x,sx,X,sX);
    for (size_t ii = 0; ii < N; ii++){
        x[ii*sX] = conj(x[ii*sX]);
        X[ii*sX] = conj(X[ii*sX]);
        X[ii*sX] /= (double)N;
    }
    return res;
}

int is2(size_t x)
{
    while (((x % 2) == 0) && x > 1) 
        x /= 2;
    return (x == 1);
}

int fft(size_t N, const double complex * x, size_t sx, 
        double complex * X, size_t sX)
{
    assert (sx == 1);
    assert (sX == 1);
    /* size_t Nopts[] = {2,4,8,16,32,64,128,256,512,1024, // 10 options */
    /*                   2048, 4096, 8192, 16384, 32768, 65536}; */

    size_t Nin = N;
    /* printf("is2=%d\n",is2(Nin)); */
    if (is2(Nin) == 0){
        fprintf(stderr,"Non-power of 2 fft is not yet implemeneted\n");
        return 1;
        /* size_t nopt = 2; */
        /* while (Nin > nopt){ */
        /*     nopt *= 2; */
        /* } */
        /* Nin = nopt; */
    }
    int res = fft_base(Nin,x,sx,X,sX);
    /* double complex * xin = malloc(Nin * sizeof(complex double)); */
    /* double complex * xout = malloc(Nin * sizeof(complex double)); */
    /* assert (xin != NULL); */
    /* assert (xout != NULL); */
    /* for (size_t ii = 0; ii < N; ii++){ */
    /*     xin[ii] = x[ii]; */
    /*     /\* xout[ii] = 0.0; *\/ */
    /* } */
    /* for (size_t ii = N; ii < Nin; ii++){ */
    /*     xin[ii] = 0.0; */
    /*     /\* xout[ii] = 0.0; *\/ */
    /* } */

    /* int res = fft_base(Nin,xin,sx,xout,sX); */
    /* if (Nin == N){ */
    /*     for (size_t ii = 0; ii < N; ii++){ */
    /*         X[ii] = xout[ii]; */
    /*     } */
    /* } */
    /* else{ */
    /*     for (size_t ii = 0; ii < N; ii++){ */
    /*         X[ii] = xout[2*ii]; */
    /*     } */
    /* } */
    
    /* /\* printf("inside\n"); *\/ */
    /* /\* for (size_t ii = 0; ii < Nin; ii++){ *\/ */
    /* /\*     printf("X[%zu] = (%G,%G)\n",ii,creal(xout[ii]),cimag(xout[ii])); *\/ */
    /* /\* } *\/ */
        
    /* free(xin); xin = NULL; */
    /* free(xout); xout = NULL; */
    return res;
}

int ifft(size_t N, double complex * x, size_t sx, 
        double complex * X, size_t sX)
{
    assert (sx == 1);
    assert (sX == 1);
    size_t Nin = N;
    if (is2(Nin) == 0){
        fprintf(stderr,"Non-power of 2 fft is not yet implemeneted\n");
        return 1;
        /* assert (sx == 1); */
        /* assert (sX == 1); */
        /* size_t nopt = 2; */
        /* while (Nin > nopt){ */
        /*     nopt *= 2; */
        /* } */
        /* Nin = nopt; */
    }
    int res = ifft_base(Nin,x,sx,X,sX);
    /* double complex * xin = malloc(Nin * sizeof(complex double)); */
    /* double complex * xout = malloc(Nin * sizeof(complex double)); */
    /* assert (xin != NULL); */
    /* assert (xout != NULL); */
    /* for (size_t ii = 0; ii < N; ii++){ */
    /*     xin[ii] = x[ii]; */
    /*     xout[ii] = 0.0; */
    /* } */
    /* for (size_t ii = N; ii < Nin; ii++){ */
    /*     xin[ii] = 0.0; */
    /*     xout[ii] = 0.0; */
    /* } */

    /* int res = ifft_base(Nin,xin,sx,xout,sX); */
    /* if (Nin == N){ */
    /*     for (size_t ii = 0; ii < N; ii++){ */
    /*         X[ii] = xout[ii]; */
    /*     } */
    /* } */
    /* else{ */
    /*     for (size_t ii = 0; ii < N; ii++){ */
    /*         X[ii] = xout[2*ii]; */
    /*     } */
    /* } */
    
    /* /\* printf("inside\n"); *\/ */
    /* /\* for (size_t ii = 0; ii < Nin; ii++){ *\/ */
    /* /\*     printf("X[%zu] = (%G,%G)\n",ii,creal(xout[ii]),cimag(xout[ii])); *\/ */
    /* /\* } *\/ */
        
    /* free(xin); xin = NULL; */
    /* free(xout); xout = NULL; */
    return res;
}
