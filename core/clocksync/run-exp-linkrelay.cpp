//
//  run-exp-linkrelay.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/12/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include "RmmApp.h"

int main(int argc, char ** argv) {
    
    RmmApp app("2001", "C0", "2020");
    app.start(false);
    app.join("54.149.44.59", "1919");
    
    while (true) {
        usleep(2000);
        app.poll();
    }
    return 0;
}