//
//  mallo.cpp
//  clocksync
//
//  Created by Zeyu Jin on 1/29/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <thread>
#include "MalloControl.h"

#define SERVER_IP_OREGON "54.149.44.59"
#define SERVER_IP_VIRGINIA "54.175.101.1"
#define SERVER_IP_TX "173.255.206.151"
#define SERVER_IP_GA "173.230.131.148"
#define SERVER_IP_UK "109.74.205.125"

#define SOURCE_PORT_DEFAULT  "1313"
#define TARGET_PORT_DEFAULT  "2323"
#define BOUNCE_PORT_DEFAULT  "4343"
#define RELAY_PORT   "1919"
#define MONITOR_PORT "2121"

#define REDIR_IP SERVER_IP_OREGON

char temp[256];

void deprecated(int argc, char ** argv) {
    printf("########################################\n");
    printf("# WELCOME TO MALLO | which is the best #\n");
    printf("########################################\n");
    
    RmmApp * app;
    
    if (argc >= 2) {
        printf("# YOUR mallo RICKNAME IS %s. CUTE!\n", argv[1]);
        app = new RmmApp(TARGET_PORT_DEFAULT, argv[1]);
    }
    else {
        printf("# Your nick name is Big-Fat-Mallo is that correct?\n(Y/N) ");
        scanf("%s", temp);
        if (temp[0] == 'n' || temp[0] == 'N') {
            printf("# Please enter your nick name: ");
            scanf("%s", temp);
        }
        printf("# Are you sure it's Big-Fat-Mallo?\n(Y/N) ");
        scanf("%s", temp);
        printf("# OK, Big-Fat-Mallo. Thanks for registration\n");
        app = new RmmApp("2323", "Big-Fat-Mallo");
    }
    
    
    std::string redirectIp = "";
    app->start(false);
    
    if (argc < 3) {
        printf("# NO REDIRECTION SPECFIED\n");
        redirectIp = SERVER_IP_GA;
        app->requestRegistration(SERVER_IP_GA, RELAY_PORT);
    }
    else {
        printf("# REDIRECT THROUGH: %s\n", argv[2]);
        app->requestRegistration(argv[2], RELAY_PORT);
        redirectIp = argv[2];
    }
    
    
    MalloControl control(app);
    
    printf("# Please wait until the other mallos reach this point...\n");
    usleep(1e6);
    
    
    printf("# Connect with whom? ");
    scanf("%s", temp);
    app->requestRedirection(redirectIp.c_str(), RELAY_PORT, temp);
    
    printf("# Connect? (Y/N)");
    scanf("%s", temp);
    
    control.connect(redirectIp.c_str(), RELAY_PORT);
    
    while (true) {
        control.poll();
        usleep(1e3);
    }
}

int main(int argc, char ** argv) {
    
    if (argc < 3) {
        printf("Usage: mallo <YourID> <TargetID>");
    }
    else {
        printf("mallo from [%s] to [%s]\n", argv[1], argv[2]);
    }
    
    std::string yourID = argv[1];
    std::string yourID_client = yourID + "-c";
    std::string yourID_server = yourID + "-s";
    
    std::string targetID = argv[2];
    std::string targetID_client = targetID + "-c";
    
    
    RmmApp * client = new RmmApp(SOURCE_PORT_DEFAULT, yourID_client.c_str(), MONITOR_PORT);
    RmmApp * server = new RmmApp(TARGET_PORT_DEFAULT, yourID.c_str(), MONITOR_PORT);
    RmmApp * server_forme = new RmmApp(BOUNCE_PORT_DEFAULT, yourID_server.c_str(), MONITOR_PORT);
    
    client->start(false);
    server->start(false);
    server_forme->start(false);
    
    lo_timetag t0;
    lo_timetag_now(&t0);
    client->alignTime(t0);
    server->alignTime(t0);
    server_forme->alignTime(t0);
    
    printf("Request registration...\n");
    client->requestRegistration(REDIR_IP, RELAY_PORT);
    server->requestRegistration(REDIR_IP, RELAY_PORT);
    server_forme->requestRegistration(REDIR_IP, RELAY_PORT);
    usleep(4e6);

    printf("Request redirection...\n");
    client->requestRedirection(REDIR_IP, RELAY_PORT, yourID_server.c_str());
    server->requestRedirection(REDIR_IP, RELAY_PORT, targetID_client.c_str());
    server_forme->requestRedirection(REDIR_IP, RELAY_PORT, yourID_client.c_str());
    usleep(4e6);
    
    
    MalloControl control(client);
    control.enableLeap();
    control.connect(REDIR_IP, RELAY_PORT);
    control.setPredictionMode(true);
    server->addmapping(1, "/hit");
    client->addmapping(1, "/hit");
    server_forme->addmapping(1, "/hit");
    
    lo_address target = lo_address_new(REDIR_IP, RELAY_PORT);
    lo_send_from(target, server->getserverptr(), LO_TT_IMMEDIATE, "/status", "");
    lo_address_free(target);
    
    while (true) {
        control.poll();
        server->poll();
        server_forme->poll();
        usleep(1e3);
    }
}