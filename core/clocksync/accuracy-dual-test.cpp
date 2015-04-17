//
//  accuracy-dual-test.cpp
//  clocksync
//
//  Created by Moonfish on 1/24/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include "accuracy-dual-test.h"

int main(int argc, char ** args) {
    
    lo_timetag t0;
    lo_timetag_now(&t0);
    
    if (argc < 2) {
        printf("Error: run-compare <target-ip>");
        RELAY_IP = SERVER_IP_GA;
    }
    else {
        RELAY_IP = args[1];
    }
    printf("\n[RUN-COMPARE-TO] %s\n", RELAY_IP);
    
    
    BounceExpr expr1("1313", "2323", 0, t0);
    BounceExpr expr2("1515", "2525", 1, t0);
    
    printf("\n[EXP STAERTED] %s\n", RELAY_IP);
    
    char temp1[255], temp2[255], fn_mallet[255], fn_midi[255];
    for (int trial = 0; trial < 10; ++ trial) {
        for (int lat = 60; lat <= 140; lat += 20) {
            printf("EXP(%d): %s starts\n", trial, fn_mallet);
            sprintf(temp1, "method1_%d_%d.txt", lat, trial);
            sprintf(temp2, "method2_%d_%d.txt", lat, trial);
            sprintf(fn_mallet, "data/%dbpm_mallet.txt", lat);
            sprintf(fn_midi, "data/%dbpm_midi.txt", lat);
            
            // run experiment
            expr1.setExperiment(fn_mallet, fn_midi);
            expr2.setExperiment(fn_mallet, fn_midi);
            
            while (true) {
                bool ret1 = expr1.poll();
                bool ret2 = expr2.poll();
                if ((ret1 || ret2) == false) break;
                usleep(500);
                
            }
            usleep(1000);
            
            
            expr1.output(temp1);
            expr2.output(temp2);
        }
    }
    
    return 0;
    
    return 0;
}
