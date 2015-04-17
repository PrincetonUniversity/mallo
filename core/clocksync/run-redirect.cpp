//
//  run-redirect.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/15/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include "MalloControl.h"

#define SOURCE_PORT "1313"
#define TARGET_PORT "2323"
#define RELAY_PORT "1919"
#define MONITOR_PORT "2121"

#define RELAY_IP "54.149.44.59"

int main() {
    
    RmmApp relay(RELAY_PORT, "R0");
    relay.start(false);
    usleep(1e6);
    
    RmmApp client(SOURCE_PORT, "CLIENT", MONITOR_PORT);
    client.start(false);
    RmmApp server(TARGET_PORT, "SERVER", MONITOR_PORT);
    server.start(false);
    client.requestRegistration(RELAY_IP, RELAY_PORT);
    server.requestRegistration(RELAY_IP, RELAY_PORT);
    usleep(1e6);
    
    client.requestRedirection(RELAY_IP, RELAY_PORT, "SERVER");
    server.requestRedirection(RELAY_IP, RELAY_PORT, "CLIENT");
    
    client.join(RELAY_IP, RELAY_PORT);
    // server.requestRegistration(RELAY2_IP, RELAY_PORT);
    // client.addRelayConnection(RELAY2_IP, RELAY_PORT);
    
    usleep(1e6);
    
    MalloControl control(&client);
    control.enableLeap();
    control.setPredictionOffset(0.02); 
    server.addmapping(1, "/hit");
    control.setPredictionMode(true);
    
    // client.addRelayConnection(RELAY_IP, RELAY_PORT);
    // server.requestRegistration(RELAY_IP, RELAY_PORT);
    
    // client.sendSched(0, 1, 100);
    
    lo_address target = lo_address_new(RELAY_IP, RELAY_PORT);
    lo_send_from(target, client.getserverptr(), LO_TT_IMMEDIATE, "/status", "");
    lo_address_free(target);
    
    while(true) {
        usleep(2000);
        control.poll();
        server.poll();
    }
    
    return 0;
}