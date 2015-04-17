//
//  MalloAlgorithms.h
//  clocksync
//
//  Created by Zeyu Jin on 1/11/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef __clocksync__MalloAlgorithms__
#define __clocksync__MalloAlgorithms__

#include <stdio.h>

void mallo_quadreg(int order, int n, double * x, double * y, double * params);

// solve b[0] + b[1]x + b[2]x^2 + ... + b[n-1]x^(n-1) = 0
double mallo_root(int n, double * b, double minv);

#endif /* defined(__clocksync__MalloAlgorithms__) */
