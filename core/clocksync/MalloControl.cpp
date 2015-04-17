//
//  MalloControl.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/11/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include "MalloControl.h"
#include "MalloAlgorithms.h"

#ifndef DISABLE_LEAP
using namespace Leap;
#endif

MalloControl::MalloControl(const char * port, const char * name) {
    node = new RmmApp(port, name);
    node->start(false);
    this->node->listener = this;
}

MalloControl::MalloControl(const char * port, const char * javaport, const char * name) {
    node = new RmmApp(port, "Mallo", javaport);
    node->start(false);
    node->addmapping(1, "/hit");
    this->node->listener = this;
}

MalloControl::MalloControl(RmmApp * app) {
    this->node = app;
    this->node->listener = this;
}

#ifndef DISABLE_LEAP
void MalloControl::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
}
void MalloControl::onFrame(const Controller& controller) {
    const Frame frame = controller.frame();
    if (frame.tools().count() > 0) {
        // printf("%.3f\napp", frame.tools()[0].tipPosition().y);
        double h = (frame.tools()[0].tipPosition().y - 80) / (double)(350 - 80);
        int x = (int)frame.tools()[0].tipPosition().x;
        printf("%d\n", x);
        input(h, x);
    }
    else if (frame.hands().count() > 0) {
        double h = frame.hand(0).finger(0).tipPosition().y + (rand() % 255) * 0.00001;
        printf("%.3f\n", h);
        input(h, 1);
    }
}
#endif

void MalloControl::enableLeap() {
#ifndef DISABLE_LEAP
    controller.addListener(*this);
    controller.setPolicy(Controller::PolicyFlag::POLICY_BACKGROUND_FRAMES);
#endif
}

void MalloControl::input(double z, int x) {
    double t = node->time();
    this->z[ptr % MalloTrackerLen] = z;
    this->t[ptr % MalloTrackerLen] = t;
    node->onSample(t, z);
    ptr = ptr + 1;
    
    if (z < 0 && t > last_local_sched) {
        RmmEvent ev; ev.time = t; ev.cmd = 1; ev.arg = x;
        // node->exec_event(ev);
        last_local_sched = t + 0.2;
    }
    
    if (ptr < 5) {
        return;
    }
    
    if (last_z < z + 0.01) {
        last_z = z;
        return;
    }
    else {
        last_z = z;
    }
    
    mallo_quadreg(regorder, ptr>MalloTrackerLen?MalloTrackerLen:ptr, this->t, this->z, this->b);
    double t0 = mallo_root(regorder + 1, this->b, t) - pred_offset;
    
    if (t0 > 1e20 || t0 < t - 0.01) {
        return;
    }
    // printf("lat = %.3f %.3f/%.3f\n", node->latencyupper(), t0, t);
    double vel = this->b[1] + t0 * 2 * this->b[2];
    
    printf("vel = %.3f\n", vel);
    
    if (contimode) {
        if (t0 - t >= 0.01 && t0 - t < 0.12) {
            sendprediction(t0 + 0.002, x, vel);
            node->onPrediction(t, t0);
        }
    }
    else {
        
        if (t0 - t < node->latencyupper() + pred_offset && t0 > _t_pred_prev + 0.2) {
            sendprediction(t0, x, vel);
            _t_pred_prev = t0;
            node->onPrediction(t, t0);
        }
    }
}

void MalloControl::connect(const char * ip, const char * port) {
    node->join(ip, port);
}

void MalloControl::onsample(double t, double y) {
    input(t, y);
}

void MalloControl::sendprediction(double t_local, int x, double vel) {
    // printf("pred: %g\n", t_local);
    node->sendSched(node->remote_time(t_local), 1, x);
}

void MalloControl::poll() {
    if (node != NULL) {
        if (useSimulation) {
            double t = node->time();
            input(sin(t * 3.1415926) * 0.5 + 0.5, 0);
        }
        node->poll();
        if (ptrMallet < mallet.size()) {
            while (ptrMallet < mallet.size() && mallet[ptrMallet].t < (node->remote_time()-sim_start)) {
                // printf("[sim]t=%f, %f\n", mallet[ptrMallet].t, mallet[ptrMallet].z);
                input(mallet[ptrMallet].z, 0);
                ptrMallet++;
            }
        }
        if (ptrHits < hits.size()) {
            while (ptrHits < hits.size() && hits[ptrHits] < (node->remote_time()-sim_start)) {
                node->onTimedEvent(node->time(), 1, 50);
                ptrHits++;
            }
        }
    }
}

void MalloControl::hit(double t) {
    
}

void MalloControl::setSimulation(bool use) {
    useSimulation = use;
}

void MalloControl::simulate(std::vector<MalloSample> mallet, std::vector<double> hits) {
    this->mallet = mallet;
    this->hits = hits;
    sim_start = node->remote_time() + 1;
    ptr = 0;
}

void MalloControl::setPredictionMode(bool usecont) {
    this->contimode = usecont;
}

void MalloControl::setPredictionOffset(double off) {
    this->pred_offset = off;
}
