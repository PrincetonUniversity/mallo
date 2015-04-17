//
//  ExpLoader.h
//  clocksync
//
//  Created by Zeyu Jin on 1/18/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef clocksync_ExpLoader_h
#define clocksync_ExpLoader_h

#include <vector>
#include <fstream>
#include "MalloControl.h"

double loadmallet(const char * fn, std::vector<MalloSample> &samples, double lo_point);
std::vector<double> loadmidi(const char * fn, double start_time);

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
        fin >> t; if (fin.eof()) break;
        fin >> ch >> note >> ch >> vel;
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
    double lastt = 0;
    while (fin.eof() == false) {
        double t, vel, note; char ch;
        fin >> t; if (fin.eof()) break;
        fin >> ch >> vel >> ch >> note;
        t -= start_time;
        if (t - lastt < 0.25) {
            continue;
        }
        hitTimes.push_back(t);
        // printf("[%.3f] vel=%d\n", t, (int)vel);
    }
    fin.close();
    return hitTimes;
}

#endif
