//
//  MalloAlgorithms.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/11/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include "MalloAlgorithms.h"
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_poly.h>

#define CI(i) (gsl_vector_get(C,(i)))
#define COV(i,j) (gsl_matrix_get(cov,(i),(j)))

void mallo_quadreg(int order, int n, double * x, double * y, double * betas) {
    
    gsl_matrix *X, *cov;
    gsl_vector *Y, *C;
    
    X = gsl_matrix_alloc (n, order + 1);
    Y = gsl_vector_alloc (n);
    
    C = gsl_vector_alloc (order + 1);
    cov = gsl_matrix_alloc (order + 1, order + 1);
    double chisq;
    
    // fill in the data
    for (int i = 0; i < n; ++i) {
        double xi = x[i];
        gsl_matrix_set (X, i, 0, 1.0);
        for (int a = 0; a < order; ++a) {
            gsl_matrix_set (X, i, a + 1, xi);
            xi = xi * xi;
        }
        gsl_vector_set (Y, i, y[i]);
    }
    gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (n, order + 1);
    gsl_multifit_linear (X, Y, C, cov, &chisq, work);
    gsl_multifit_linear_free (work);
    
    for (int a = 0; a < order + 1; ++a) {
        betas[a] = CI(a);
    }
    gsl_matrix_free (X);
    gsl_vector_free (Y);
    gsl_vector_free (C);
    gsl_matrix_free (cov);
}

double mallo_root(int n, double * b, double minv) {
    gsl_poly_complex_workspace * w = gsl_poly_complex_workspace_alloc (n);
    double z[24];
    gsl_poly_complex_solve (b, n, w, z);
    gsl_poly_complex_workspace_free (w);
    double val = 1e100;
    for (int i = 0; i < n - 1; i++) {
        
        if (z[2*i+1] < 1e-8 && z[2*i+1] > -1e-8) {
            if (val > z[2*i] && z[2*i] >= minv) {
                val = z[2*i];
            }
        }
    }
    return val;
}
