//
//  RmmApp.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/10/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include "RmmApp.h"

RmmApp::RmmApp(const char * port, const char * name) : RmmNode(port, name) {
    muteLocalapp = true;
}

RmmApp::RmmApp(const char * port, const char * name, const char * portLocalapp) : RmmNode(port, name) {
    muteLocalapp = false;
    this->portLocalapp = portLocalapp;
}

void RmmApp::addmapping(RmmAction cmd, const char *sendingmsg) {
    RmmEventMap map;
    map.cmd = cmd;
    map.message = sendingmsg;
    maplist.push_back(map);
}

void RmmApp::onLatency(double owt, double rtt, double upper) {
    if (muteLocalapp == false) {
        lo_address localapp = lo_address_new("localhost", portLocalapp.c_str());
        // send to local app
        int ret = lo_send_from(localapp, NULL, LO_TT_IMMEDIATE, "/latt", "ddd", owt, rtt, upper);
        if (ret < 0)
            printf("(%s-APP) latency message sending FAILED(%s)\n", this->nodeId.c_str(), portLocalapp.c_str());
        lo_address_free(localapp);
    }
}

void RmmApp::onClockDiff(double t, double tdiff) {
    if (muteLocalapp == false) {
        lo_address localapp = lo_address_new("localhost", portLocalapp.c_str());
        // send to local app
        int ret = lo_send_from(localapp, NULL, LO_TT_IMMEDIATE, "/clocksync", "dd", t, tdiff);
        if (ret < 0)
            printf("(%s-APP) clock message sending FAILED(%s)\n", this->nodeId.c_str(), portLocalapp.c_str());
        lo_address_free(localapp);
    }
}


void RmmApp::onChat(const char *content) {
}

void RmmApp::onTimedEvent(double t, int cmd, int arg) {
    for (int a = 0; a < maplist.size(); ++a) {
        if (maplist[a].cmd == cmd) {
            // send the event
            //printf("[%s-APP] %s", this->nodeId.c_str(), maplist[a].message.c_str());
            printf("%.3f\n", t);
            if (!muteLocalapp) {
                lo_address localapp = lo_address_new("localhost", portLocalapp.c_str());
                // send to local app
                int ret = lo_send_from(localapp, NULL, LO_TT_IMMEDIATE, maplist[a].message.c_str(), "di", t, arg);
                if (ret < 0)
                    printf(" FAILED(%s)\n", portLocalapp.c_str());
                // else
                    // printf(" SENT\n");
                lo_address_free(localapp);
            }
            else {
                printf("\n");
            }
        }
    }

}

void RmmApp::onEvent(int cmd, int arg) {
    for (int a = 0; a < maplist.size(); ++a) {
        if (maplist[a].cmd == cmd) {
            // send the event
            printf("[%s-APP] %s", this->nodeId.c_str(), maplist[a].message.c_str());
            if (!muteLocalapp) {
                lo_address localapp = lo_address_new("localhost", portLocalapp.c_str());
                // send to local app
                int ret = lo_send_from(localapp, NULL, LO_TT_IMMEDIATE, maplist[a].message.c_str(), "i", arg);
                if (ret < 0)
                    printf(" FAILED(%s)\n", portLocalapp.c_str());
                else
                    printf(" SENT\n");
                lo_address_free(localapp);
            }
            else {
                printf("\n");
            }
        }
    }

}

void RmmApp::onPrediction(double t, double t0) {
    if (!muteLocalapp) {
        lo_address localapp = lo_address_new("localhost", portLocalapp.c_str());
        // send to local app
        // printf("[%s-APP] pred-at %.3f for %.3f\n", this->nodeId.c_str(), t, t0);
        int ret = lo_send_from(localapp, NULL, LO_TT_IMMEDIATE, "/pred", "dd", t, t0);
        if (ret < 0)
            printf("[%s-APP] prediction message FAILED(port:%s)\n", this->nodeId.c_str(), portLocalapp.c_str());
        lo_address_free(localapp);

    }
}

void RmmApp::onSample(double t0, double z) {
    if (!muteLocalapp) {
        lo_address localapp = lo_address_new("localhost", portLocalapp.c_str());
        // send to local app
        int ret = lo_send_from(localapp, NULL, LO_TT_IMMEDIATE, "/smp", "dd", t0, z);
        if (ret < 0)
            printf("[%s-APP] sample message FAILED(port:%s)\n", this->nodeId.c_str(), portLocalapp.c_str());
        lo_address_free(localapp);
    }
}

void RmmApp::onInput(double t0, double z) {
    if (listener != NULL) {
        listener->onsample(t0, z);
    }
}
