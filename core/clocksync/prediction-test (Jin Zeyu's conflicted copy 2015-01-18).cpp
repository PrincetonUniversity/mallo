//
//  prediction-test.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/16/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>

#include "MalloControl.h"

class PredictTest : public MalloPredictListener {
    double latency;
    std::vector<double> scheduledHits;
protected:
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
public:
    virtual void predictionFound(double t, double t_predd) {
        // actual time = t + latency;
        t = t + latency;
        if (t > t_predd) {
            printf("scheduled %.3f\n", t);
            scheduledHits.push_back(t);
        }
        else {
            printf(": %.3f\n", t);
        }
    }
    virtual void sampleSent() {
        printf(":\n");
    }
public:
    void runTest(const char * fn_mallet, const char * fn_midi, double lo_point, double latency) {
        this->latency = latency;
        MalloPredictor predictor;
        predictor.setListene
        r(this);
        
        std::vector<MalloSample> samples;
        double t0 = loadmallet(fn_mallet, samples, lo_point);
        std::vector<double> hits = loadmidi(fn_midi, t0);
        
        for (int a = 0; a < samples.size(); ++a) {
            predictor.input(samples[a].z, samples[a].t);
            predictor.predict(latency, samples[a].t);
        }
    }
};



int main() {
    
    PredictTest test;
    test.runTest( "data/60bpm_mallet.txt", "data/60bpm_midi.txt", 0, 0.05);
    
    return 0;
    
}
