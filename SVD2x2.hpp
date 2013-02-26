//============================================================================
// Name        : SVD.cpp
// Author      : George Rokos
// Description : SVD solver for 2x2 linear systems - definitions
//============================================================================

#ifndef SVD2X2_HPP_
#define SVD2X2_HPP_

#ifdef __CUDAC__
#define HOST __host__
#define DEVICE __device__
#define GLOBAL __global__
#else
#define HOST
#define DEVICE
#define GLOBAL
#endif


void DEVICE svd_solve_2x2(const double *A, double *p, const double *q);

#endif /* SVD_HPP_ */
