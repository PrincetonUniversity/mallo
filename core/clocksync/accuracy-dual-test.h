//
//  accuracy-dual-test.h
//  clocksync
//
//  Created by Moonfish on 1/24/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef __clocksync__accuracy_dual_test__
#define __clocksync__accuracy_dual_test__

#include <stdio.h>
#include <stdlib.h>

#include "ExpLoader.h"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include "MalloControl.h"

#define SERVER_IP_OREGON "54.149.44.59"
#define SERVER_IP_VIRGINIA "54.175.101.1"
#define SERVER_IP_TX "173.255.206.151"
#define SERVER_IP_GA "173.230.131.148"
#define SERVER_IP_UK "109.74.205.125"
#define SERVER_IP_RACK "104.130.127.193"

#define SOURCE_PORT_DEFAULT  "1313"
#define TARGET_PORT_DEFAULT  "2323"
#define RELAY_PORT   "1919"
#define MONITOR_PORT "2121"

const char * RELAY_IP;

// "54.149.44.59"
// "54.149.44.59"
// #define RELAY2_IP "54.175.101.1"
// "54.175.101.1"

class BounceExpr : public TestListener {
protected:
    RmmApp * client;
    RmmApp * server;
    MalloControl * control;
    
    std::string serverId;
    
    std::vector<std::pair<double, double>> predictions, locs;
    std::vector<double> scheduledHits;
    std::vector<double> messageLatencies;
    std::vector<double> hits;
    std::vector<MalloSample> samples;
    
public:
    virtual void onExec(double t, double t_sched, double t_pred_send, double t_pred_recv) {
        printf("<EXE=%.3f,SCHED=%.3f,%.3f->%.3f> lat = %.3f\n", t, t_sched, t_pred_send, t_pred_recv, t_pred_recv - t_pred_send);
        scheduledHits.push_back(t);
        std::pair<double, double> prd(t_pred_send, t_sched);
        predictions.push_back(prd);
        messageLatencies.push_back(t_pred_recv - t_pred_send);
    }
    virtual void onPredRecved(double lat, const char * src, bool firstarrival) {
        
    }
    BounceExpr(const char * SOURCE_PORT, const char * TARGET_PORT, int algorithm, lo_timetag tag) {
        // Create local relay
        client = new RmmApp(SOURCE_PORT, SOURCE_PORT);
        server = new RmmApp(TARGET_PORT, TARGET_PORT);
        serverId = TARGET_PORT;
        usleep(1e6);
        client->start(false);
        server->start(false);
        client->setForceNoClocksync(true);
        server->setForceNoClocksync(true);
        client->alignTime(tag);
        server->alignTime(tag);
        client->requestRegistration(RELAY_IP, RELAY_PORT);
        server->requestRegistration(RELAY_IP, RELAY_PORT);
        usleep(1e6);
        
        
        
        // ask the relay to redirect
        client->requestRedirection(RELAY_IP, RELAY_PORT, TARGET_PORT);
        server->requestRedirection(RELAY_IP, RELAY_PORT, SOURCE_PORT);
        client->join(RELAY_IP, RELAY_PORT);
        
        usleep(1e6);
        
        control = new MalloControl(client);
        control->enableLeap();
        control->setPredictionOffset(0.001);
        if (algorithm == 0) {
            control->setPredictionMode(true);
        }
        else if (algorithm == 1) {
            control->setPredictionMode(false);
        }
        else {
            
        }
        server->addmapping(1, "/hit"); //
        
        lo_address target = lo_address_new(RELAY_IP, RELAY_PORT);
        lo_send_from(target, client->getserverptr(), LO_TT_IMMEDIATE, "/status", "");
        lo_address_free(target);
        
        server->setTestListener(this);
        
        usleep(1e6);
    }
    
    int ptr = 0;
    double t_begin = 0;
    
    void setExperiment(const char * fn_mallo, const char * fn_hits) {
        
        predictions.clear();
        scheduledHits.clear();
        messageLatencies.clear();
        locs.clear();
        samples.clear();
        
        double t_lo = loadmallet(fn_mallo, samples, 105);
        hits = loadmidi(fn_hits, t_lo);
        
        t_begin = server->remote_time() + 1;
        for (int a = 0; a < hits.size(); ++a) {
            hits[a] += t_begin;
        }
        
        ptr = 0;
        
    }
    
    bool poll() {
        if (ptr >= samples.size()) {
            return false;
        }
        double t = client->time();
        if (t > samples[ptr].t + t_begin) {
            // printf("%s %.3f\n", serverId.c_str(), samples[ptr].z);
            std::pair<double, double> malloloc;
            malloloc.first = samples[ptr].t + t_begin;
            malloloc.second = samples[ptr].z;
            control->input(samples[ptr].z, 0);
            ptr = ptr + 1;
            locs.push_back(malloloc);
        }
        server->poll();
        control->poll();

        return true;
    }
    
    void output(const char * fn) {
        FILE * pf = fopen(fn, "w");
        
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
        
        for (int a = 0; a < locs.size(); ++a) {
            fprintf(pf, "%.3f ", locs[a].first);
        }
        fprintf(pf, "\n");
        
        for (int a = 0; a < locs.size(); ++a) {
            fprintf(pf, "%.3f ", locs[a].second);
        }
        fprintf(pf, "\n");
        for (int a = 0; a < messageLatencies.size(); ++a) {
            fprintf(pf, "%.3f ", messageLatencies[a]);
        }
        
        
        fclose(pf);;
    }
    
};

#endif /* defined(__clocksync__accuracy_dual_test__) */
