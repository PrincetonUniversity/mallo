//
//  run-bounce-exp.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/18/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

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

#define RELAY_IP SERVER_IP_RACK
// "54.149.44.59"
// "54.149.44.59"
// #define RELAY2_IP "54.175.101.1"
// "54.175.101.1"


class BounceExpr : public TestListener {
protected:
    RmmApp * relay;
    RmmApp * client;
    RmmApp * server;
    MalloControl * control;
    
    std::vector<std::pair<double, double>> predictions, locs;
    std::vector<double> scheduledHits;
    std::vector<double> messageLatencies;
    std::vector<double> hits;
    
public:
    virtual void onExec(double t, double t_sched, double t_pred_send, double t_pred_recv) {
        printf("<%.3f,%.3f,%.3f> lat = %.3f\n", t, t_sched, t_pred_send, t_pred_recv - t_pred_send);
        scheduledHits.push_back(t);
        std::pair<double, double> prd(t_pred_send, t_sched);
        predictions.push_back(prd);
        messageLatencies.push_back(t_pred_recv - t_pred_send);
    }
    virtual void onPredRecved(double lat, const char * src, bool firstarrival) {
        
    }
    BounceExpr(const char * SOURCE_PORT, const char * TARGET_PORT) {
        // Create local relay
        relay = new RmmApp(RELAY_PORT, "R0");
        client = new RmmApp(SOURCE_PORT, "CLIENT", MONITOR_PORT);
        server = new RmmApp(TARGET_PORT, "SERVER", MONITOR_PORT);
        client->setForceNoClocksync(true);
        server->setForceNoClocksync(true);
        
        relay->start(false);
        usleep(1e6);
        client->start(false);
        server->start(false);
        client->requestRegistration(RELAY_IP, RELAY_PORT);
        server->requestRegistration(RELAY_IP, RELAY_PORT);
        usleep(1e6);
        
        // ask the relay to redirect
        client->requestRedirection(RELAY_IP, RELAY_PORT, "SERVER");
        server->requestRedirection(RELAY_IP, RELAY_PORT, "CLIENT");
        client->join(RELAY_IP, RELAY_PORT);
        
        usleep(1e6);
        
        control = new MalloControl(client);
        control->enableLeap();
        control->setPredictionOffset(0.001);
        control->setPredictionMode(true);
        server->addmapping(1, "/hit"); //
        
        lo_address target = lo_address_new(RELAY_IP, RELAY_PORT);
        lo_send_from(target, client->getserverptr(), LO_TT_IMMEDIATE, "/status", "");
        lo_address_free(target);
        
        server->setTestListener(this);
        
        usleep(1e6);
    }
    
    void runExperiment(const char * fn_mallo, const char * fn_hits) {
        
        std::vector<MalloSample> samples;
        predictions.clear();
        scheduledHits.clear();
        messageLatencies.clear();
        locs.clear();
        
        double t_lo = loadmallet(fn_mallo, samples, 105);
        hits = loadmidi(fn_hits, t_lo);
        
        double t_begin = client->time() + 1;
        for (int a = 0; a < hits.size(); ++a) {
            hits[a] += t_begin;
        }
        
        int ptr = 0;
        while (ptr < samples.size()) {
            double t = client->time();
            if (t > samples[ptr].t + t_begin) {
                // printf("IN %.3f", samples[ptr].z);
                std::pair<double, double> malloloc;
                malloloc.first = samples[ptr].t + t_begin;
                malloloc.second = samples[ptr].z;
                control->input(samples[ptr].z, 0);
                ptr = ptr + 1;
                locs.push_back(malloloc);
            }
            server->poll();
            control->poll();
            usleep(500);
        }
        usleep(1000);
        
        printf("Experiment for %s is done.\n", fn_mallo);
        
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



int main() {
    BounceExpr expr("1313", "2323");
    char temp[255], fn_mallet[255], fn_midi[255];
    for (int trial = 0; trial < 20; ++ trial) {
        for (int lat = 60; lat <= 140; lat += 20) {
            sprintf(temp, "output_%d_%d.txt", lat, trial);
            sprintf(fn_mallet, "data/%dbpm_mallet.txt", lat);
            sprintf(fn_midi, "data/%dbpm_midi.txt", lat);
            expr.runExperiment(fn_mallet, fn_midi);
            expr.output(temp);
        }
    }
    
    return 0;
}