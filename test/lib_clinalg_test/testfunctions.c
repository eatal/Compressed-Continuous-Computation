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

// first function

#include "testfunctions.h"
#include <assert.h>
#include <math.h>

int func(size_t n, const double * x, double * out, void * args)
{
    for (size_t ii = 0; ii < n; ii++){
        out[ii] = 1.0 + 0.0*x[ii];
    }
    if (args != NULL){
        int * N = args;
        *N += n;
    }
    return 0;
}

int func2(size_t n, const double * x, double * out, void * args)
{
    for (size_t ii = 0; ii < n; ii++){
        out[ii] = x[ii];
    }
    if (args != NULL){
        int * N = args;
        *N += n;
    }
    return 0;
}

int func3(size_t n, const double * x, double * out, void * args)
{
    for (size_t ii = 0; ii < n; ii++){
        out[ii] = pow(x[ii],2.0) + sin(3.14159*x[ii]);
    }
    if (args != NULL){
        int * N = args;
        *N += n;
    }
    return 0;
}

int func4(size_t n, const double * x, double * out, void * args)
{
    for (size_t ii = 0; ii < n; ii++){
        out[ii] = 3.0*pow(x[ii],4.0) - 2.0*pow(x[ii],2.0);
    }
    if (args != NULL){
        int * N = args;
        *N += n;
    }
    return 0;
}

int func5(size_t n, const double * x, double * out, void * args)
{
    for (size_t ii = 0; ii < n; ii++){
        //return 3.0*cos(M_PI*x) - 2.0*pow(x,0.5);
        out[ii] = x[ii];
    }
    if (args != NULL){
        int * N = args;
        *N += n;
    }
    return 0;
}

int func6(size_t n, const double * x, double * out, void * args)
{
    for (size_t ii = 0; ii < n; ii++){
        out[ii] = exp(5.0*x[ii]);
    }
    if (args != NULL){
        int * N = args;
        *N += n;
    }
    return 0;
}

