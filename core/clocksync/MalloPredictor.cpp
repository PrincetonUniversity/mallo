//
//  MalloPredictor.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/16/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include "MalloPredictor.h"

#include "MalloAlgorithms.h"

void MalloPredictor::reset() {
    ptr = 0;
}

void MalloPredictor::predict(double upperbound, double t) {
    
    if (ptr < 5) {
        // printf("< 5\n");
        return;
    }
    double z = this->z[(ptr - 1) % MalloTrackerLen];
    double last_z = this->z[(ptr - 2) % MalloTrackerLen];
    double wrapz = this->z[(ptr) % MalloTrackerLen];
    if (last_z < z) {
        // printf("smz\n");
        return;
    }
    
    mallo_quadreg(regorder, ptr>MalloTrackerLen?MalloTrackerLen:ptr, this->t, this->z, this->b);
    double t0 = mallo_root(regorder + 1, this->b, t) - pred_offset;
    
    
    if (t0 > 1e20 || t0 < t - 0.02) {
        return;
    }
    
    // printf("%.3f + %.3fx = 0\n", this->b[0], this->b[1]);
    
    if (contimode) {
        if (t0 - t >= 0 && t0 - t < 0.1) {
            if (listener != NULL) {
                listener->predictionFound(t, t0);
            }
            // sendprediction(t0, x);
            // node->onPrediction(t, t0);
        }
    }
    else {
        
        if (t0 - t < upperbound + pred_offset && t0 > _t_pred_prev + 0.2) {
            if (listener != NULL) {
                listener->predictionFound(t, t0);
            }
            _t_pred_prev = t0;
            // sendprediction(t0, x);
            // node->onPrediction(t, t0);
        }
    }
}

void MalloPredictor::input(double t, double z) {
    this->z[ptr % MalloTrackerLen] = z;
    this->t[ptr % MalloTrackerLen] = t;
    ptr = ptr + 1;
    if (listener != NULL) {
        listener->sampleSent(t, z);
    }
}

void MalloPredictor::setContMode(bool enable) {
    this->contimode = enable;
}

void MalloPredictor::setListener(MalloPredictListener *listener) {
    this->listener = listener;
}

void MalloPredictor::setPredOffset(double off) {
    this->pred_offset = off;
}


