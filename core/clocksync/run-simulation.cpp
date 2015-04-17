//
//  run-simulation.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/12/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include "MalloControl.h"
#include "MalloAlgorithms.h"


double loadmallet(const char * fn, std::vector<MalloSample> &samples, double lo_point) {
    std::ifstream fin(fn);
    if (fin.is_open() == false) {
        exit(-1);
    }
    std::string pop;
    fin >> pop >> pop >> pop;
    
    double t0 = 0;
    double hmax = 0;
    while (fin.eof() == false) {
        MalloSample smp;
        double t, vel, note; char ch;
        fin >> t >> ch >> note >> ch >> vel;
        if (samples.size() == 0) {
            t0 = t;
        }
        t -= t0;
        smp.t = t;
        smp.z = vel;
        if (hmax < vel) {
            hmax = vel;
        }
        samples.push_back(smp);
        // printf("[%.3f] z=%d\n", t, (int)vel);
    }
    
    for (int a = 0; a < samples.size(); ++a) {
        samples[a].z = (samples[a].z - lo_point) / (hmax - lo_point);
    }
    fin.close();
    return t0;
}

std::vector<double> loadmidi(const char * fn, double start_time) {
    std::ifstream fin(fn);
    if (fin.is_open() == false) {
        exit(-1);
    }
    std::string pop;
    fin >> pop >> pop >> pop;
    std::vector<double> hitTimes;
    while (fin.eof() == false) {
        double t, vel, note; char ch;
        fin >> t >> ch >> vel >> ch >> note;
        t -= start_time;
        hitTimes.push_back(t);
        // printf("[%.3f] vel=%d\n", t, (int)vel);
    }
    fin.close();
    return hitTimes;
}

void runTest(MalloControl &mallo, const char * fn_mallet, const char * fn_midi, double lo_point) {
    std::vector<MalloSample> samples;
    double t0 = loadmallet(fn_mallet, samples, lo_point);
    std::vector<double> hits = loadmidi(fn_midi, t0);
    mallo.simulate(samples, hits);
}

int main (void)
{
    // RmmApp remote("1818", "S1");
    // remote.start(false);
    MalloControl mallo("1515", "2121");
    mallo.connect("54.149.44.59", "1919");
    
    for (int t = 0; t < 5000; ++t) {
        mallo.poll();
        usleep(1000);
    }
    runTest(mallo, "data/60bpm_mallet.txt", "data/60bpm_midi.txt", 110);
    while (true) {
        mallo.poll();
        usleep(1000);
    }
    return 0;
}