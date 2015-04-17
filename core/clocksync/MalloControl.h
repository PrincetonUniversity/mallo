//
//  MalloControl.h
//  clocksync
//
//  Created by Zeyu Jin on 1/11/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef __clocksync__MalloControl__
#define __clocksync__MalloControl__

#include <stdio.h>
#include <vector>

#include <gsl/gsl_fit.h>

#include "MalloPredictor.h"
#include "RmmApp.h"

#ifndef DISABLE_LEAP
#include "Leap.h"
#endif

#define MalloTrackerLen 7

struct MalloSample {
    double t, z;
};

class MalloControl : public
#ifndef DISABLE_LEAP
Leap::Listener,
#endif
MalloPredictor, ControllerListener  {
    
    bool contimode = false;
    RmmApp * node = NULL;
#ifndef DISABLE_LEAP
    Leap::Controller controller;
#endif
    
    // simulation
    std::vector<MalloSample> mallet;
    std::vector<double> hits;
    int ptrHits = 0, ptrMallet = 0;
    double sim_start = 0;
    bool useSimulation = false;
    
    double pred_offset = 0;
    
    double last_z = 0;
    double last_local_sched = 0;
    
    
protected:
    void sendprediction(double t_local, int x, double vel);
    
public:
    MalloControl(RmmApp * app);
    MalloControl(const char * port, const char * name);
    MalloControl(const char * port, const char * javaport, const char * name);
    void enableLeap();
    
    void setPredictionOffset(double off);
    
    void connect(const char * ip, const char * port);
    void input(double z, int x);
    void hit(double t);
    void setSimulation(bool use);
    void simulate(std::vector<MalloSample> mallet, std::vector<double> hits);
    void poll();
    void setPredictionMode(bool usecont);
    
    virtual void onsample(double t, double y);
    
protected:
    int countFrames = 0;
    
#ifndef DISABLE_LEAP
public:
    virtual void onConnect(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
#endif
};

#endif /* defined(__clocksync__MalloControl__) */
