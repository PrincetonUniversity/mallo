//
//  MalloPredictor.h
//  clocksync
//
//  Created by Zeyu Jin on 1/16/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef __clocksync__MalloPredictor__
#define __clocksync__MalloPredictor__

#include <stdio.h>

#define MAX_MalloTrackerLen 42

class MalloPredictListener {
public:
    virtual void predictionFound(double t, double t_predd) = 0;
    virtual void sampleSent(double t, double z) = 0;
};

class MalloPredictor {
    MalloPredictListener * listener = NULL;
protected:
    int MalloTrackerLen = 5;
    int ptr = 0;
    int regorder = 2;
    double last_z;
    double _t_pred_prev = 0;
    double z[MAX_MalloTrackerLen];
    double t[MAX_MalloTrackerLen];
    double b[10];
    double pred_offset;
    
    bool contimode = false;
    
public:
    void setListener(MalloPredictListener * listener);
    void predict(double upperbound, double t);
    void input(double t, double z);
    void reset();
    void setPredOffset(double off);
    void setContMode(bool enable);
};

#endif /* defined(__clocksync__MalloPredictor__) */
