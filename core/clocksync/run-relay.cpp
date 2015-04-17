//
//  relay.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/12/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "RmmApp.h"

int main(int argc, char ** argv) {
    
    std::string port;
    if (argc < 2) {
        printf("usage: ./mallo-relay <port>");
        port = "1919";
    }
    else {
        port = argv[1];
        printf("port is %s\n", port.c_str());
    }
    
    std::string name = "R0";
    if (argc >= 3) {
        name = argv[2];
    }
    
    float serverdelay = 0;
    if (argc >= 4) {
        serverdelay = (argv[3][0] - '0') * 5;
        printf("Delay added: %f", serverdelay);
    }
    
    
    RmmApp app(port.c_str(), name.c_str());
    app.setRelayDelay(serverdelay);
    
    printf("Relay established: %s\n", name.c_str());
    app.start(false);
    
    int counter = 0;
    while (true) {
        app.poll();
        usleep(2000);
        if (counter ++ % 500 == 0) {
            app.handleAlive();
        }
    }
    
    return 0;
}