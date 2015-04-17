//
//  run-overlay.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/15/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>

#include "RmmApp.h"

#define SERVER_IP_OREGON "54.149.44.59"

#define SERVER_IP_VIRGINIA "54.175.101.1"
#define SERVER_IP_TX "173.255.206.151"
#define SERVER_IP_GA "173.230.131.148"
#define SERVER_IP_UK "109.74.205.125"

#define SOURCE_PORT  "1313"
#define TARGET_PORT  "2323"
#define RELAY_PORT   "1919"
#define MONITOR_PORT "2121"

#define SERVER_NAME "54.149.44.59:1919"

#define SERVER_IP SERVER_IP_OREGON

struct RelayStat {
    double lat;
    bool first;
    int source = -1;
};

class RelayExpr : public TestListener {
    
protected:
    RmmApp * relay;
    RmmApp * client;
    RmmApp * server;
    
    std::vector<RelayStat> stats;
    
public:
    virtual void onExec(double t, double t_sched, double t_pred_send, double t_pred_recv) {
    }
    
    virtual void onPredRecved(double lat, const char * src, bool firstarrival) {
        std::string srcip = src;
        RelayStat st; st.lat = lat;
        st.first = firstarrival;
        if (src == SERVER_IP_VIRGINIA) {
            st.source = 1;
        } else if (src == SERVER_IP_TX) {
            st.source = 2;
        } else if (src == SERVER_IP_GA) {
            st.source = 3;
        } else if (src == SERVER_IP_UK) {
            st.source = 4;
        }
        stats.push_back(st);
        printf("PRED-RECV: %s latency=%.3f\n", src, st.lat);
    }
    
    RelayExpr() {
        // Create local relay
        relay  = new RmmApp(RELAY_PORT, "R0");
        client = new RmmApp(SOURCE_PORT, "CLIENT", MONITOR_PORT);
        
        relay->start(false);
        client->start(false);
        usleep(1e6);
        
        client->requestRegistration(SERVER_IP_VIRGINIA, RELAY_PORT);
        usleep(1e6);
        
        // ask the relay to redirect
        client->join(SERVER_IP, RELAY_PORT);
        
        usleep(1e6);
        
        lo_address target = lo_address_new(SERVER_IP, RELAY_PORT);
        lo_send_from(target, client->getserverptr(), LO_TT_IMMEDIATE, "/status", "");
        lo_address_free(target);
        
        client->setTestListener(this);
        client->changeTargetName(SERVER_NAME);
        client->addRelayConnection(SERVER_IP_VIRGINIA, RELAY_PORT);
        client->addRelayConnection(SERVER_IP_TX, RELAY_PORT);
        client->addRelayConnection(SERVER_IP_GA, RELAY_PORT);
        client->addRelayConnection(SERVER_IP_UK, RELAY_PORT);
        
        for (int a = 0; a < 1000; ++a) {
            usleep(1000);
            client->poll();
            relay->poll();
        }
    }
    
    void runExperiment(int count) {
        
        for (int a = 0; a < count; ++a) {
            client->poll();
            client->sendSched(client->time() + 0.5, 1, 100);
            usleep(0.25e6);
        }
        
    }
    
    void output(const char * fn) {
        
    }
    
};

int main() {
    RelayExpr expr;
    expr.runExperiment(2000);
    
    return 0;
}
