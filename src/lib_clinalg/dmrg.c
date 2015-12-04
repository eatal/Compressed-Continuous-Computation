// Copyright (c) 2014-2015, Massachusetts Institute of Technology
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

/** \file dmrg.c
 * Provides routines for dmrg based algorithms for the FT
 */

#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "array.h"
#include "lib_linalg.h"
#include "lib_clinalg.h"

/***********************************************************//**
    Perform and store a reduced QR or LQ decompositions

    \param a [in] - qmarray to decompose
    \param type [in] - 1 then QR, 0 then LQ

    \return qr - QR structure

    \note
        Doesnt actually do reduced QR because it seems to break the algorithm
***************************************************************/
struct QR * qr_reduced(struct Qmarray * a, int type)
{
    struct Qmarray * ac = qmarray_copy(a);
    struct QR * qr = NULL;
    if ( NULL == (qr = malloc(sizeof(struct QR)))){
        fprintf(stderr, "failed to allocate memory for QR decomposition.\n");
        exit(1);
    }
    qr->mat = NULL;
    qr->Q = NULL;
    if (type == 0){
        qr->right = 0;
        qr->mr = a->nrows;
        qr->mc = a->nrows;
        int success = qmarray_lq(ac,&(qr->Q),&(qr->mat));
        assert (success == 0);
    }
    else if (type == 1){
        qr->right = 1;
        qr->mc = a->ncols;
        qr->mr = a->ncols;
        int success = qmarray_qr(ac,&(qr->Q),&(qr->mat));
        assert (success == 0);
    }
    else{
        fprintf(stderr, "Can't take reduced qr decomposition of type %d\n",type);
        exit(1);
    }
    qmarray_free(ac); ac = NULL;
    return qr;

}

/***********************************************************//**
    Free memory allocated to QR decomposition

    \param qr [inout] - QR structure to free
***************************************************************/
void qr_free(struct QR * qr)
{
    if (qr != NULL){
        free(qr->mat); qr->mat = NULL;
        qmarray_free(qr->Q); qr->Q = NULL;
        free(qr); qr = NULL;
    }
}

/***********************************************************//**
    Allocate memory for an array of QR structures
    
    \param n [in] - size of array

    \return qr - array of QR structure (each of which is set to NULL)
***************************************************************/
struct QR ** qr_array_alloc(size_t n)
{
    struct QR ** qr = NULL;
    if ( NULL == (qr = malloc(n*sizeof(struct QR *)))){
        fprintf(stderr, "failed to allocate memory for QR array.\n");
        exit(1);
    }
    size_t ii;
    for (ii = 0; ii < n; ii++){
        qr[ii] = NULL;
    }
    return qr;
}


/***********************************************************//**
    Free memory allocated to array of QR structures

    \param qr [inout] - array to free
***************************************************************/
void qr_array_free(struct QR ** qr, size_t n)
{
    if (qr != NULL){
        size_t ii;
        for (ii = 0; ii < n; ii++){
            qr_free(qr[ii]); qr[ii] = NULL;
        }
        free(qr); qr = NULL;
    }
}


/***********************************************************//**
    Update right side component of supercore for FT product

    \param a [in] - core 1
    \param b [in] - core 2
    \param prev [in] - previous right side to update
    \param z [in] - right multiplier
    
    \return lq - new right side component
***************************************************************/
struct QR * dmrg_super_rprod(struct Qmarray * a, struct Qmarray * b,
                             struct QR * prev, struct Qmarray * z)
{

    double * val = qmaqmat_integrate(prev->Q,z);
    struct Qmarray * temp = qmarray_kron_mat(z->nrows,val,a,b);
    //struct Qmarray * temp = qmam(nextleft,val,z->nrows);
    struct QR * lq = qr_reduced(temp,0);
    qmarray_free(temp); temp = NULL;
    free(val); val = NULL;
    return lq;
}

/***********************************************************//**
    Update all right side component of supercore for FT product

    \param a [in] - first ft in product
    \param b [in] - second ft in product
    \param z [in] - guess
    
    \return lq - new right side component
***************************************************************/
struct QR ** 
dmrg_super_rprod_all(
    struct FunctionTrain * a, struct FunctionTrain * b,
    struct FunctionTrain * z)
{
    size_t dim = a->dim;
    struct QR ** right = NULL;
    if ( NULL == (right = malloc((dim-1)*sizeof(struct QR *)))){
        fprintf(stderr, "failed to allocate memory for dmrg all right QR decompositions.\n");
        exit(1);
    }
    struct Qmarray * temp = qmarray_kron(a->cores[dim-1],b->cores[dim-1]);
    right[dim-2] = qr_reduced(temp,0);
    qmarray_free(temp);

    int ii;
    for (ii = dim-3; ii > -1; ii--){
        //printf("on core %d\n",ii);
        right[ii] = dmrg_super_rprod(a->cores[ii+1],b->cores[ii+1],
                                        right[ii+1],
                                        z->cores[ii+2]);
    }
    return right;
}

/***********************************************************//**
    Perform a left-right dmrg sweep for FT-product

    \param z [in] - initial guess
    \param a [in] - first element of product
    \param b [in] - second element of product
    \param phil [inout] - left multipliers
    \param psir [in] - right multiplies
    \param epsilon [in] - splitting tolerance

    \return na - a new approximation
***************************************************************/
struct FunctionTrain * 
dmrg_sweep_lr_prod(struct FunctionTrain * z, 
            struct FunctionTrain * a, struct FunctionTrain * b,
            struct QR ** phil, struct QR ** psir, double epsilon)
{
    double * RL = NULL;
    size_t dim = z->dim;
    struct FunctionTrain * na = function_train_alloc(dim);
    na->ranks[0] = 1;
    na->ranks[na->dim] = 1;
    
    
    if (phil[0] == NULL){
        struct Qmarray * temp0 = qmarray_kron(a->cores[0],b->cores[0]);
        phil[0] = qr_reduced(temp0,1);
        qmarray_free(temp0); temp0 = NULL;
    }
    
    size_t nrows = phil[0]->mr;
    size_t nmult = phil[0]->mc;
    size_t ncols = psir[0]->mc;

    RL = calloc_double(nrows * ncols);
    cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                nmult,1.0,phil[0]->mat,nrows,psir[0]->mat,nmult,0.0,RL,nrows);

    double * u = NULL;
    double * vt = NULL;
    double * s = NULL;
    //printf("(nrows,ncols)=(%zu,%zu)\n",nrows,ncols);
    size_t rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
    na->ranks[1] = rank;
    na->cores[0] = qmam(phil[0]->Q,u,rank);
    
    size_t ii;
    for (ii = 1; ii < dim-1; ii++){
        double * newphi = calloc_double(rank * phil[ii-1]->mc);
        cblas_dgemm(CblasColMajor,CblasTrans,CblasNoTrans,rank,nmult,
                    nrows,1.0,u,nrows,phil[ii-1]->mat,nrows,0.0,newphi,rank);
        //struct Qmarray * temp = mqma(newphi,y->cores[ii],rank);
        struct Qmarray * temp = qmarray_mat_kron(rank,newphi,a->cores[ii],b->cores[ii]);

        qr_free(phil[ii]); phil[ii] = NULL;
        phil[ii] = qr_reduced(temp,1);
        
        free(RL); RL = NULL;
        free(newphi); newphi = NULL;
        free(u); u = NULL;
        free(vt); vt = NULL;
        free(s); s = NULL;
        qmarray_free(temp); temp = NULL;

        nrows = phil[ii]->mr;
        nmult = phil[ii]->mc;
        ncols = psir[ii]->mc;

        RL = calloc_double(nrows * ncols);
        cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                    nmult,1.0,phil[ii]->mat,nrows,psir[ii]->mat,nmult,0.0,RL,nrows);

        rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
        na->ranks[ii+1] = rank;
        na->cores[ii] = qmam(phil[ii]->Q,u,rank);
    }
    
    size_t kk,jj;
    for (kk = 0; kk < ncols; kk++){
        for (jj = 0; jj < rank; jj++){
            vt[kk*rank+jj] = vt[kk*rank+jj]*s[jj];
        }
    }
    
    
    na->cores[dim-1] = mqma(vt,psir[dim-2]->Q,rank);

    free(RL); RL = NULL;
    free(u); u = NULL;
    free(vt); vt = NULL;
    free(s); s = NULL;

    return na;
}

/***********************************************************//**
    Perform a right-left dmrg sweep as part of ft-product

    \param z [in] - initial guess
    \param a [in] - first element of product
    \param b [in] - second element of product
    \param phil [inout] - left multipliers
    \param psir [in] - right multiplies
    \param epsilon [in] - splitting tolerance

    \return na - a new approximation
***************************************************************/
struct FunctionTrain * 
dmrg_sweep_rl_prod(struct FunctionTrain * z, 
            struct FunctionTrain * a, struct FunctionTrain * b,
            struct QR ** phil, struct QR ** psir, double epsilon)
{
    double * RL = NULL;
    size_t dim = z->dim;
    struct FunctionTrain * na = function_train_alloc(dim);
    na->ranks[0] = 1;
    na->ranks[na->dim] = 1;
    
       
    if (psir[dim-2] == NULL){
        struct Qmarray * temp0 = qmarray_kron(a->cores[dim-1],b->cores[dim-1]);
        psir[dim-2] = qr_reduced(temp0,0);
        qmarray_free(temp0); temp0 = NULL;
    }
    
    size_t nrows = phil[dim-2]->mr;
    size_t nmult = phil[dim-2]->mc;
    size_t ncols = psir[dim-2]->mc;

    RL = calloc_double(nrows * ncols);
    cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                nmult,1.0,phil[dim-2]->mat,nrows,psir[dim-2]->mat,nmult,0.0,RL,nrows);

    double * u = NULL;
    double * vt = NULL;
    double * s = NULL;
    size_t rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
    na->ranks[dim-1] = rank;
    na->cores[dim-1] = mqma(vt,psir[dim-2]->Q,rank); 
    
    int ii;
    for (ii = dim-3; ii > -1; ii--){
        double * newpsi = calloc_double( psir[ii+1]->mr * rank);
        //
        cblas_dgemm(CblasColMajor,CblasNoTrans,CblasTrans,
                    psir[ii+1]->mr,rank, psir[ii+1]->mc,
                    1.0,psir[ii+1]->mat,psir[ii+1]->mr,vt,rank,
                    0.0,newpsi,psir[ii+1]->mr);

        struct Qmarray * temp = qmarray_kron_mat(rank,newpsi,a->cores[ii+1],b->cores[ii+1]);

        qr_free(psir[ii]); psir[ii] = NULL;
        psir[ii] = qr_reduced(temp,0);
        
        free(RL); RL = NULL;
        free(newpsi); newpsi = NULL;
        free(u); u = NULL;
        free(vt); vt = NULL;
        free(s); s = NULL;
        qmarray_free(temp); temp = NULL; 
        nrows = phil[ii]->mr;
        nmult = phil[ii]->mc;
        ncols = psir[ii]->mc;

        RL = calloc_double(nrows * ncols);
        cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                    nmult,1.0,phil[ii]->mat,nrows,psir[ii]->mat,nmult,0.0,RL,nrows);

        rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
        na->ranks[ii+1] = rank;
        na->cores[ii+1] = mqma(vt,psir[ii]->Q,rank); 
    }
    
    size_t kk,jj;
    for (jj = 0; jj < rank; jj++){
        for (kk = 0; kk < nrows; kk++){
            u[jj*nrows+kk] = u[jj*nrows+kk]*s[jj];
        }
    }
    
    na->cores[0] = qmam(phil[0]->Q,u,rank);

    free(RL); RL = NULL;
    free(u); u = NULL;
    free(vt); vt = NULL;
    free(s); s = NULL;

    return na;
}

/***********************************************************//**
    Perform a left-right-left dmrg sweep for FT product

    \param z [in] - initial guess
    \param a [in] - first element of product
    \param b [in] - second element of product
    \param phil [inout] - left multipliers
    \param psir [inout] - right multiplies
    \param epsilon [in] - splitting tolerance

    \return na - a new approximation
***************************************************************/
struct FunctionTrain * 
dmrg_sweep_lrl_prod(struct FunctionTrain * z,
                struct FunctionTrain * a, struct FunctionTrain * b, 
                struct QR ** phil, struct QR ** psir, double epsilon)
{

    struct FunctionTrain * temp = dmrg_sweep_lr_prod(z,a,b,phil,psir,epsilon);
    struct FunctionTrain * na = dmrg_sweep_rl_prod(temp,a,b,phil,psir,epsilon);
    function_train_free(temp); temp = NULL;
    return na;
}

/***********************************************************//**
    Compute \f$ z(x) = a(x)b(x) \f$ using ALS+DMRG

    \param z [inout] - initial guess (destroyed);
    \param a [in] - first element of product
    \param b [in] - second element of product
    \param delta [in] - threshold to stop iterating
    \param max_sweeps [in] - maximum number of left-right-left sweeps 
    \param epsilon [in] - SVD tolerance for rank determination
    \param verbose [in] - verbosity level 0 or >0

***************************************************************/
struct FunctionTrain * dmrg_product(struct FunctionTrain * z,
            struct FunctionTrain * a, struct FunctionTrain * b,
            double delta, size_t max_sweeps, double epsilon, int verbose)
{
    size_t dim = a->dim;
    struct FunctionTrain * na = function_train_orthor(z);

    struct QR ** psir = dmrg_super_rprod_all(a,b,na);
    struct QR ** phil = qr_array_alloc(dim-1);
    
    size_t ii;
    double diff;
    for (ii = 0; ii < max_sweeps; ii++){
        if (verbose>0){
            printf("On Sweep (%zu/%zu) \n",ii+1,max_sweeps);
        }
        struct FunctionTrain * check = dmrg_sweep_lrl_prod(na,a,b,phil,psir,epsilon);
        diff = function_train_relnorm2diff(check,na);
        function_train_free(na); na = NULL;
        na = function_train_copy(check);
        function_train_free(check); check = NULL;
        
        if (verbose > 0){
            printf("\t The relative error between approximations is %G\n",diff);
        }
        if (diff < delta){
            break;
        }
    }

    qr_array_free(psir,dim-1);
    qr_array_free(phil,dim-1);
    return na;
}


/***********************************************************//**
    Update right side component of supercore

    \param nextleft [in] - next left multiplier
    \param prev [in] - previous right side to update
    \param z [in] - right multiplier
    
    \return lq - new right side component
***************************************************************/
struct QR *
dmrg_update_right2(struct Qmarray * nextleft, struct QR * prev, struct Qmarray * z)
{

    double * val = qmaqmat_integrate(prev->Q,z);
    struct Qmarray * temp = qmam(nextleft,val,z->nrows);
    struct QR * lq = qr_reduced(temp,0);
    qmarray_free(temp); temp = NULL;
    free(val); val = NULL;
    return lq;
}



struct QR ** 
dmrg_update_right2_all(struct FunctionTrain * y, struct FunctionTrain * z)
{
    size_t dim = y->dim;
    struct QR ** right = NULL;
    if ( NULL == (right = malloc((dim-1)*sizeof(struct QR *)))){
        fprintf(stderr, "failed to allocate memory for dmrg all right QR decompositions.\n");
        exit(1);
    }
    right[dim-2] = qr_reduced(y->cores[dim-1],0);
    int ii;
    for (ii = dim-3; ii > -1; ii--){
        //printf("on core %d\n",ii);
        right[ii] = dmrg_update_right2(y->cores[ii+1], 
                                        right[ii+1],
                                        z->cores[ii+2]);
    }
    return right;
}



/***********************************************************//**
    Perform a left-right dmrg sweep

    \param z [in] - initial guess
    \param y [in] - desired ft
    \param phil [inout] - left multipliers
    \param psir [in] - right multiplies
    \param epsilon [in] - splitting tolerance

    \return na - a new approximation
***************************************************************/
struct FunctionTrain * 
dmrg_sweep_lr2(struct FunctionTrain * z, struct FunctionTrain * y, 
            struct QR ** phil, struct QR ** psir, double epsilon)
{
    double * RL = NULL;
    size_t dim = z->dim;
    struct FunctionTrain * na = function_train_alloc(dim);
    na->ranks[0] = 1;
    na->ranks[na->dim] = 1;
    
    
    if (phil[0] == NULL){
        phil[0] = qr_reduced(y->cores[0],1);
    }
    
    size_t nrows = phil[0]->mr;
    size_t nmult = phil[0]->mc;
    size_t ncols = psir[0]->mc;

    RL = calloc_double(nrows * ncols);
    cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                nmult,1.0,phil[0]->mat,nrows,psir[0]->mat,nmult,0.0,RL,nrows);

    double * u = NULL;
    double * vt = NULL;
    double * s = NULL;
    size_t rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
    na->ranks[1] = rank;
    na->cores[0] = qmam(phil[0]->Q,u,rank);
    
    size_t ii;
    for (ii = 1; ii < dim-1; ii++){
        double * newphi = calloc_double(rank * phil[ii-1]->mc);
        cblas_dgemm(CblasColMajor,CblasTrans,CblasNoTrans,rank,nmult,
                    nrows,1.0,u,nrows,phil[ii-1]->mat,nrows,0.0,newphi,rank);
        struct Qmarray * temp = mqma(newphi,y->cores[ii],rank);

        qr_free(phil[ii]); phil[ii] = NULL;
        phil[ii] = qr_reduced(temp,1);
        
        free(RL); RL = NULL;
        free(newphi); newphi = NULL;
        free(u); u = NULL;
        free(vt); vt = NULL;
        free(s); s = NULL;
        qmarray_free(temp); temp = NULL;

        nrows = phil[ii]->mr;
        nmult = phil[ii]->mc;
        ncols = psir[ii]->mc;

        RL = calloc_double(nrows * ncols);
        cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                    nmult,1.0,phil[ii]->mat,nrows,psir[ii]->mat,nmult,0.0,RL,nrows);

        rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
        na->ranks[ii+1] = rank;
        na->cores[ii] = qmam(phil[ii]->Q,u,rank);
    }
    
    size_t kk,jj;
    for (kk = 0; kk < ncols; kk++){
        for (jj = 0; jj < rank; jj++){
            vt[kk*rank+jj] = vt[kk*rank+jj]*s[jj];
        }
    }
    
    
    na->cores[dim-1] = mqma(vt,psir[dim-2]->Q,rank);

    free(RL); RL = NULL;
    free(u); u = NULL;
    free(vt); vt = NULL;
    free(s); s = NULL;

    return na;
}

/***********************************************************//**
    Perform a right-left dmrg sweep

    \param z [in] - initial guess
    \param y [in] - desired ft
    \param phil [inout] - left multipliers
    \param psir [in] - right multiplies
    \param epsilon [in] - splitting tolerance

    \return na - a new approximation
***************************************************************/
struct FunctionTrain * 
dmrg_sweep_rl2(struct FunctionTrain * z, struct FunctionTrain * y, 
            struct QR ** phil, struct QR ** psir, double epsilon)
{
    double * RL = NULL;
    size_t dim = z->dim;
    struct FunctionTrain * na = function_train_alloc(dim);
    na->ranks[0] = 1;
    na->ranks[na->dim] = 1;
    
       
    if (psir[dim-2] == NULL){
        psir[dim-2] = qr_reduced(y->cores[dim-1],0);
    }
    
    size_t nrows = phil[dim-2]->mr;
    size_t nmult = phil[dim-2]->mc;
    size_t ncols = psir[dim-2]->mc;

    RL = calloc_double(nrows * ncols);
    cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                nmult,1.0,phil[dim-2]->mat,nrows,psir[dim-2]->mat,nmult,0.0,RL,nrows);

    double * u = NULL;
    double * vt = NULL;
    double * s = NULL;
    size_t rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
    na->ranks[dim-1] = rank;
    na->cores[dim-1] = mqma(vt,psir[dim-2]->Q,rank); 
    
    int ii;
    for (ii = dim-3; ii > -1; ii--){
        double * newpsi = calloc_double( psir[ii+1]->mr * rank);
        //
        cblas_dgemm(CblasColMajor,CblasNoTrans,CblasTrans,
                    psir[ii+1]->mr,rank, psir[ii+1]->mc,
                    1.0,psir[ii+1]->mat,psir[ii+1]->mr,vt,rank,
                    0.0,newpsi,psir[ii+1]->mr);

        struct Qmarray * temp = qmam(y->cores[ii+1],newpsi,rank);

        qr_free(psir[ii]); psir[ii] = NULL;
        psir[ii] = qr_reduced(temp,0);
        
        free(RL); RL = NULL;
        free(newpsi); newpsi = NULL;
        free(u); u = NULL;
        free(vt); vt = NULL;
        free(s); s = NULL;
        qmarray_free(temp); temp = NULL; 
        nrows = phil[ii]->mr;
        nmult = phil[ii]->mc;
        ncols = psir[ii]->mc;

        RL = calloc_double(nrows * ncols);
        cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,nrows,ncols,
                    nmult,1.0,phil[ii]->mat,nrows,psir[ii]->mat,nmult,0.0,RL,nrows);

        rank = truncated_svd(nrows,ncols,nrows,RL,&u,&s,&vt,epsilon);
        na->ranks[ii+1] = rank;
        na->cores[ii+1] = mqma(vt,psir[ii]->Q,rank); 
    }
    
    size_t kk,jj;
    for (jj = 0; jj < rank; jj++){
        for (kk = 0; kk < nrows; kk++){
            u[jj*nrows+kk] = u[jj*nrows+kk]*s[jj];
        }
    }
    
    na->cores[0] = qmam(phil[0]->Q,u,rank);

    free(RL); RL = NULL;
    free(u); u = NULL;
    free(vt); vt = NULL;
    free(s); s = NULL;

    return na;
}

/***********************************************************//**
    Perform a left-right-left dmrg sweep

    \param a [in] - initial guess
    \param b [in] - desired ft
    \param phil [inout] - left multipliers
    \param psir [inout] - right multiplies
    \param epsilon [in] - splitting tolerance

    \return na - a new approximation
***************************************************************/
struct FunctionTrain * 
dmrg_sweep_lrl2(struct FunctionTrain * a, struct FunctionTrain * b, 
                struct QR ** phil, struct QR ** psir, double epsilon)
{

    struct FunctionTrain * temp = dmrg_sweep_lr2(a,b,phil,psir,epsilon);
    struct FunctionTrain * na = dmrg_sweep_rl2(temp,b,phil,psir,epsilon);
    function_train_free(temp); temp = NULL;
    return na;
}

/***********************************************************//**
    Find an approximation of an FT with another FT through dmrg sweeps

    \param z [inout] - initial guess (destroyed);
    \param y [in] - desired ft
***************************************************************/
struct FunctionTrain * dmrg_approx2(struct FunctionTrain * z, struct FunctionTrain * y,
            double delta, size_t max_sweeps, int verbose, double epsilon)
{
    size_t dim = y->dim;
    struct FunctionTrain * na = function_train_orthor(z);
    struct QR ** psir = dmrg_update_right2_all(y,na);
    struct QR ** phil = qr_array_alloc(dim-1);
    
    size_t ii;
    double diff;
    for (ii = 0; ii < max_sweeps; ii++){
        if (verbose>0){
            printf("On Sweep (%zu/%zu) \n",ii+1,max_sweeps);
        }
        struct FunctionTrain * check = dmrg_sweep_lrl2(na,y,phil,psir,epsilon);
        diff = function_train_relnorm2diff(check,na);
        function_train_free(na); na = NULL;
        na = function_train_copy(check);
        function_train_free(check); check = NULL;
        
        if (verbose > 0){
            printf("\t The relative error between approximations is %G\n",diff);
        }
        if (diff < delta){
            break;
        }
    }

    qr_array_free(psir,dim-1);
    qr_array_free(phil,dim-1);
    return na;
}

