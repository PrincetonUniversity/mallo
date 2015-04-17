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
    std::vector<std::pair<double, double>> predictions;
    std::vector<double> scheduledHits;
    std::vector<double> hits;
    double sched_exec_t = 0, sched_at_t = 0;
    
    int nFalsePositives = 0;
    int nTrueNegatives = 0;
    double meanError = 0;
    
    bool verbose = false;
    bool doOneStepLater = false;
    
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
            if (smp.t < 0) {
                continue;
            }
            samples.push_back(smp);
            
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
        double lastt = -1;
        std::string pop;
        fin >> pop >> pop >> pop;
        std::vector<double> hitTimes;
        while (fin.eof() == false) {
            double t, vel, note; char ch;
            fin >> t >> ch >> vel >> ch >> note;
            t -= start_time;
            if (t < 0) {
                continue;
            }
            if (t > lastt + 0.2)
                hitTimes.push_back(t);
            lastt = t;
            if (verbose)
                printf("[%.3f] vel=%d\n", t, (int)vel);
        }
        fin.close();
        return hitTimes;
    }
private:
    double onestep_last_pred = 0;
    bool onesteplate = false;
    
public:
    virtual void predictionFound(double t, double t_predd) {
        // actual time = t + latency;
        // printf("[SCHD] %6.3f | exec.%6.3f\n", t, t_predd);
        
        if (doOneStepLater && t_predd < onestep_last_pred) {
            onesteplate = true;
        }
        
        onestep_last_pred = t_predd;
        sched_at_t = t;
        sched_exec_t = t_predd;
    }
    virtual void sampleSent(double t, double z) {
        // printf(":%.3f z=%.3f\n",t,z);
    }
protected:
    void evaluate() {
        int truthFoundpair[255];
        for (int a = 0; a < hits.size(); ++a) truthFoundpair[a] = -1;
        int predFoundpair[255];
        for (int a = 0; a < scheduledHits.size(); ++a) predFoundpair[a] = -1;
        double dfamount = 0;
        int dfcount = 0;
        for (int a = 0; a < hits.size(); ++a) {
            double mindist = 1;
            int minIdx = -1;
            for (int b = 0; b < scheduledHits.size(); ++b) {
                if (predFoundpair[b] == -1) {
                    double df = scheduledHits[b] - hits[a] > 0? scheduledHits[b] - hits[a] : -(scheduledHits[b] - hits[a]);
                    if (df < 0.05) {
                        if (mindist > df) {
                            mindist = df;
                            minIdx = b;
                        }
                    }
                }
            }
            if (minIdx == -1) {
                // truthFoundpair[a] = -1;
                // printf("[%.3f] cannot find pair\n", hits[a]);
            }
            else {
                // printf("[%.3f] found %.3f[%d]\n", hits[a], scheduledHits[minIdx], minIdx);
                predFoundpair[minIdx] = a;
                truthFoundpair[a] = minIdx;
                dfamount = dfamount + mindist;
                dfcount = dfcount + 1;
            }
        }
        int fp = 0;
        for (int a = 0; a < hits.size(); ++a) {
            if (truthFoundpair[a] == -1) fp++;
        }
        int tn = 0;
        for (int a = 0; a < scheduledHits.size(); ++a) {
            if (predFoundpair[a] == -1) {
                tn ++;
                // printf("%.3f false positive\n", scheduledHits[a]);
            }
        }
        dfamount = dfamount / (double)dfcount;
        printf("mean = (%5.2fms) falsepos = %d, trueneg = %d\n", dfamount * 1000, fp, tn);
        nFalsePositives = fp;
        nTrueNegatives = tn;
        meanError = dfamount;
    }
public:
    void outputdata(const char * fn) {
        
    }
    void runTest(MalloPredictor &predictor, const char * fn_mallet, const char * fn_midi, double lo_point, double latency) {
        this->latency = latency;
        
        predictor.setListener(this);
        
        std::vector<MalloSample> samples;
        double t0 = loadmallet(fn_mallet, samples, lo_point);
        hits = loadmidi(fn_midi, t0);
        
        std::string fn = fn_mallet; fn = fn + ".txt";
        
        std::ifstream fin(fn.c_str());
        
        int input_ptr = 0;
        double time = 0;
        while (input_ptr < samples.size()) {
            if (time >= samples[input_ptr].t) {
                predictor.input(samples[input_ptr].t, samples[input_ptr].z);
                input_ptr ++;
            }
            predictor.predict(latency + 0.01, time);
            double t = time + latency;
            if (sched_exec_t > 0 && ((t >= sched_exec_t && !onesteplate) || (onesteplate && t >= sched_exec_t + 0.01))) {
                if (verbose)
                    printf("[EXEC] %6.3f\n", t);
                sched_exec_t = 0;
                std::pair<double, double> pred;
                pred.first = sched_at_t; pred.second = sched_exec_t;
                predictions.push_back(pred); // the prediction being executed
                scheduledHits.push_back(t);
                time = time + 0.2;
            }
            else {
                // printf("[EXEC] %6.3f\n", t);
            }
            time = time + 0.001;
        }
        evaluate();
        
        FILE * pf = fopen("out.txt", "w");
        
        for (int a = 0; a < hits.size(); ++a) {
            fprintf(pf, "%.3f ", hits[a]);
        }
        fprintf(pf, "\n");
        
        for (int a = 0; a < predictions.size(); ++a) {
            fprintf(pf, "%.3f ", predictions[a].first);
        }
        fprintf(pf, "\n");
        
        for (int a = 0; a < scheduledHits.size(); ++a) {
            fprintf(pf, "%.3f ", scheduledHits[a]);
        }
        fprintf(pf, "\n");
        
        for (int a = 0; a < samples.size(); ++a) {
            fprintf(pf, "%.3f ", samples[a].t);
        }
        fprintf(pf, "\n");
        
        for (int a = 0; a < samples.size(); ++a) {
            fprintf(pf, "%.3f ", samples[a].z);
        }
        
        fclose(pf);;
    }
    void doVerbose() {
        verbose = true;
    }
    void doOneStepLaterAlgorithm() {
        doOneStepLater = true;
    }
};

void multitest() {
    std::vector<PredictTest> testConst;
    for (double latc = 0.02; latc < 0.14; latc+=0.01) {
        PredictTest test;
        MalloPredictor predictor;
        test.runTest(predictor, "data/60bpm_mallet.txt", "data/60bpm_midi.txt", 105, latc);
        testConst.push_back(test);
    }
    std::vector<PredictTest> testVar;
    for (double latc = 0.02; latc < 0.14; latc+=0.01) {
        PredictTest test;
        MalloPredictor predictor;
        predictor.setContMode(true);
        test.runTest(predictor, "data/60bpm_mallet.txt", "data/60bpm_midi.txt", 105, latc);
        testConst.push_back(test);
    }
}


int main() {
    
    PredictTest test;
    MalloPredictor predictor;
    test.doVerbose();
    // test.doOneStepLaterAlgorithm();
    // predictor.setContMode(true);
    test.runTest(predictor, "data/140bpm_mallet.txt", "data/140bpm_midi.txt", 105, 0.050);
    
    return 0;
    
}
