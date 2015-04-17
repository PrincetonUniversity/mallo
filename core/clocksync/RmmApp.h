//
//  RmmApp.h
//  clocksync
//
//  Created by Zeyu Jin on 1/10/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef __clocksync__RmmApp__
#define __clocksync__RmmApp__

#include <stdio.h>
#include "RmmNode.h"

typedef int RmmAction;

struct RmmEventMap {
    int cmd;
    std::string message;
};

struct ControllerListener {
    virtual void onsample(double t, double y) = 0;
};


class RmmApp : public RmmNode {
    std::vector<RmmEventMap> maplist;
    std::string portLocalapp;
    bool muteLocalapp = true;
    lo_server_thread serverlocalapp;
public:
    ControllerListener * listener = NULL;
    RmmApp(const char * port, const char * name);
    RmmApp(const char * port, const char * name, const char * local);
    void addmapping(RmmAction cmd, const char * sendingmsg);
    virtual void onChat(const char * content);
    virtual void onEvent(int cmd, int arg);
    virtual void onLatency(double owt, double rtt, double upper);
    virtual void onTimedEvent(double t, int cmd, int arg);
    virtual void onPrediction(double t, double t0);
    virtual void onSample(double t0, double z);
    virtual void onInput(double t0, double z);
    virtual void onClockDiff(double t, double tdiff);
};

#endif /* defined(__clocksync__RmmApp__) */
