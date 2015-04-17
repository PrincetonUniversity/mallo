//
//  MalloClient.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/11/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "MalloControl.h"
#include "MalloAlgorithms.h"

void testpoly() {
    double x[4] = { 1, 2, 3, 4 };
    double y[4] = { 4, 6, 10, 16};
    double b[3] = { 0, 0, 0 };
    mallo_quadreg(2, 4, x, y, b);
    
    b[0] = -2;
    
    printf("y = %g + %gx + %gx^2\n", b[0], b[1], b[2]);
    double v = mallo_root(3, b, 105);
    printf("(%g, 0)\n", v);
}

int main (void)
{
    // RmmApp remote("1818", "S1");
    // remote.start(false);
    MalloControl mallo("1515", "2020");
    mallo.connect("54.149.44.59", "1919");
    mallo.setSimulation(true);
    while (true) {
        mallo.poll();
        usleep(20000);
    }
    return 0;
}