//
//  main.cpp
//  clocksync
//
//  Created by Zeyu Jin on 11/25/14.
//  Copyright (c) 2014 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>

#include "RmmApp.h"

int main() {
    RmmApp relay("4664", "R0");
    relay.start(false);
    
    RmmApp server("7676", "SERVER", "1919");
    RmmApp client("2424", "CLIENT", "1919");
    
    server.start(false);
    server.setDebug(true);
    server.setForcedTimeOffset(10); // is 10 seconds ahead of the client
    server.addmapping(1, "/hit");
    server.requestRegistration("localhost", "4664");
    
    client.start(false);
    client.join("localhost", "7676");
    client.addRelayConnection("localhost", "4664");
    
    lo_timetag t0, t1;
    lo_timetag_now(&t0);
    
    int counter = 0;
    
    while (true) {
        lo_timetag_now(&t1);
        // sprintf(temp, "t=%.2fs", lo_timetag_diff(t0, t1));
        server.poll();
        client.poll();
        relay.poll();
        usleep(1000);
        // node2.sendChat(temp);
        if (counter++ % 2000 == 0) {
            client.sendSched(client.remote_time() + 0.1, 1, 3);
        }
        
    }
    
    return 0;
}

