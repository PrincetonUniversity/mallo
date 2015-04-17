//
//  RmmNode.cpp haha
//  clocksync
//
//  Created by Zeyu Jin on 1/6/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#include "RmmNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


void error(int num, const char *m, const char *path);

int echo_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data);

int quit_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data);


////////////////////
///////// DEF of RmmNode
////////////////////


RmmNode::RmmNode(const char * port, const char * nodeId) {
    this->localPort = port;
    this->nodeId = nodeId;
    this->connectedTo.status = RMM_OFFLINE;
}

void RmmNode::restart(const char * port, bool useTcp) {
    // turn off UDP connection
    
    std::string portStr = port;
    if (inited && portStr == this->localPort) {
        // no need to restart
        return;
    }
    else {
        localPort = portStr;
    }
    
    lo_timetag_now(&t0);
    
    inited = false;
    st = lo_server_thread_new_with_proto(localPort.c_str(), useTcp ? LO_TCP : LO_UDP, error);
    if (!st) {
        printf("Could not create server thread.\n");
        exit(1);
    }
    server = lo_server_thread_get_server(st);
    
    /* add method that will match the path /quit with no args */
    lo_server_thread_add_method(st, "/quit", "", quit_handler, NULL);
    
    /* add method that will match any path and args */
    lo_server_thread_add_method(st, NULL, NULL, echo_handler, this);
    
    lo_server_thread_start(st);
    
    printf("[%s-localh] node created %s, TCP = %d\n", nodeId.c_str(), lo_server_get_url(server), useTcp);
    
    inited = true;
}

void RmmNode::handleJoin(lo_address target, const char * cid) {
    
    const char *host = lo_address_get_hostname(target);
    const char *port = lo_address_get_port(target);
    addClient(host, port, cid);
    
    
    printf("[%s-localh] [%s] requested to join, (%s:%s)\n", this->nodeId.c_str(), cid, host, port);
    lo_send_from(target, server, LO_TT_IMMEDIATE, "/joined", "s", nodeId.c_str());
}

void RmmNode::addClient(const char *host, const char *port, const char *cid) {
    RmmConn connection;
    connection.remoteIp = host;
    connection.remotePort = port;
    connection.nodeId = cid;
    connection.checkTime = time();
    for (int a = 0; a < connections.size(); ++a) {
        if (connections[a].remotePort == connection.remotePort && connections[a].remoteIp == connection.remoteIp) {
            connections[a] = connection;
            return;
        }
    }
    connections.push_back(connection);
}

void RmmNode::handleAlive(lo_address target) {
    std::string host = lo_address_get_hostname(target);
    std::string port = lo_address_get_port(target);
    
    double t = time();
    for (int a = 0; a < connections.size(); ++a) {
        if (host == connections[a].remoteIp && port == connections[a].remotePort) {
            connections[a].checkTime = t;
            connections[a].status = RMM_ALIVE;
        }
        if (t - connections[a].checkTime > 10) {
            connections[a].status = RMM_OUTOFSYNC;
            needCleanUpConnectionList = true;
        }
    }
    
    for (int a = 0; a < redirectlist.size(); ++a) {
        if (host == redirectlist[a].remoteIp && port == redirectlist[a].remotePort) {
            redirectlist[a].checkTime = t;
            redirectlist[a].status = RMM_ALIVE;
        }
        if (t - redirectlist[a].checkTime > 10) {
            redirectlist[a].status = RMM_OUTOFSYNC;
            needCleanUpRedirectList = true;
        }
    }
}

void RmmNode::handleAlive() {
    double t = time();
    for (int a = 0; a < connections.size(); ++a) {
        if (t - connections[a].checkTime > 10) {
            connections[a].status = RMM_OUTOFSYNC;
            needCleanUpConnectionList = true;
        }
    }
    
    for (int a = 0; a < redirectlist.size(); ++a) {
        if (t - redirectlist[a].checkTime > 10) {
            redirectlist[a].status = RMM_OUTOFSYNC;
            needCleanUpRedirectList = true;
        }
    }
}

void RmmNode::requestRedirection(const char *host, const char *port, const char * target) {
    lo_address relay = lo_address_new(host, port);
    int ret = lo_send_from(relay, server, LO_TT_IMMEDIATE, "/redirect", "s", target);
    if (ret < 0) {
        printf("[%s-localh] ERROR in sending /register %s\n", nodeId.c_str(), nodeId.c_str());
    }
    lo_address_free(relay);
}

void RmmNode::requestRegistration(const char *host, const char *port) {
    lo_address relay = lo_address_new(host, port);
    int ret = lo_send_from(relay, server, LO_TT_IMMEDIATE, "/register", "s", this->nodeId.c_str());
    if (ret < 0) {
        printf("[%s-localh] ERROR in sending /register %s\n", nodeId.c_str(), nodeId.c_str());
    }
    lo_address_free(relay);
}

void RmmNode::start(bool useTcp) {
    if (inited) {
        if (useTcp != this->useTcp) {
            close();
            clientReset();
        }
        else {
            return; // no need to restart the server
        }
    }
    this->useTcp = useTcp;
    restart(this->localPort.c_str(), useTcp);
}

void RmmNode::close() {
    lo_server_thread_stop(this->st);
    lo_server_thread_free(this->st);
    lo_server_free(this->server);
}

void RmmNode::addRelayConnection(const char *ip, const char *port) {
    RmmConn conn;
    relayConnections.push_back(conn);
    int index = (int)relayConnections.size() - 1;
    relayConnections[index].remoteIp = ip;
    relayConnections[index].remotePort = port;
    relayConnections[index].remote = lo_address_new_with_proto(useTcp?LO_TCP:LO_UDP, ip, port);
    if (!relayConnections[index].remote) {
        printf("[%s:CLIENT] Error creating destination address.\n", this->nodeId.c_str());
    }
    else {
        
    }
    printf("[%s-CLIENT] RELAY %s:%s\n", this->nodeId.c_str(), relayConnections[index].remoteIp.c_str(), relayConnections[index].remotePort.c_str());
}

void RmmNode::changeTargetName(const char * name) {
    connectedTo.nodeId = name;
}

double RmmNode::time() {
    lo_timetag t1;
    lo_timetag_now(&t1);
    return lo_timetag_diff(t1, t0) + forcedoffset;
}

double RmmNode::remote_time() {
    lo_timetag t1;
    lo_timetag_now(&t1);
    return lo_timetag_diff(t1, t0) + forcedoffset + (forceNoClockSync ? 0 : timeOffset);
}

double RmmNode::remote_time(double t) {
    return t + timeOffset;
}

bool RmmNode::isDebug() {
    return _debug;
}

void RmmNode::alignTime(lo_timetag tag) {
    this->t0 = tag;
}

void RmmNode::multisend(lo_address target, lo_message &msg, const char * path) {
    lo_send_message_from(client, server, path, msg);
}

//// SEND**** FUNCTIONS

void RmmNode::sendChat(const char * content) {
    if (clientInited) {
        lo_send_from(client, server, LO_TT_IMMEDIATE, "/chat", "s", content);
    }
}

void RmmNode::sendSched(double t_ms, int midiCode, int content) {
    if (clientInited) {
        lo_send_from(client, server, LO_TT_IMMEDIATE, "/sched", "ddiis", remote_time(), t_ms, midiCode, content, this->nodeId.c_str());
        for (int a = 0; a < relayConnections.size(); ++a) {
            // lo_address relay = lo_address_new_with_proto(useTcp?LO_TCP:LO_UDP, relayConnections[a].remoteIp.c_str(), relayConnections[a].remotePort.c_str());
            std::string path = "/sched";
            int err = lo_send_from(relayConnections[a].remote, server, LO_TT_IMMEDIATE, "/fwd", "ssddiis", path.c_str(), connectedTo.nodeId.c_str(), remote_time(), t_ms, midiCode, content, this->nodeId.c_str());
            if (err < 0) {
                printf("[%s-CLIENT] cannot access relay %d\n", this->nodeId.c_str(), err);
            }
            
            // lo_address_free(relay);
        }
    }
}

void RmmNode::sendPred(double t_ms, int midiCode, int content) {
    if (clientInited) {
        lo_send_from(client, server, LO_TT_IMMEDIATE, "/pred", "ddiis", remote_time(), t_ms, midiCode, content, this->nodeId.c_str());
        for (int a = 0; a < relayConnections.size(); ++a) {
            // lo_address relay = lo_address_new_with_proto(useTcp?LO_TCP:LO_UDP, relayConnections[a].remoteIp.c_str(), relayConnections[a].remotePort.c_str());
            std::string path = "/pred";
            int err = lo_send_from(relayConnections[a].remote, server, LO_TT_IMMEDIATE, "/fwd", "ssddiis", path.c_str(), connectedTo.nodeId.c_str(), remote_time(), t_ms, midiCode, content, this->nodeId.c_str());
            if (err < 0) {
                printf("[%s-CLIENT] cannot access relay %d\n", this->nodeId.c_str(), err);
            }
            
            // lo_address_free(relay);
        }
    }
}

void RmmNode::sendClockRequest() {
    _recsynctime = time();
    if (clientInited) {
        lo_send_from(client, server, LO_TT_IMMEDIATE, "/syncreq", "d", _recsynctime);
    }
}

void RmmNode::sendLatencyTest() {
    _lattesttime = remote_time();
    if (clientInited) {
        lo_send_from(client, server, LO_TT_IMMEDIATE, "/lattreq", "d", _recsynctime);
    }
}

////////// handle*** functions

void RmmNode::handleClockRequest(lo_address target) {
    lo_send_from(target, server, LO_TT_IMMEDIATE, "/clock", "d", time());
}

void RmmNode::handleClockRequest(lo_address target, double simlat) {
    lo_send_from(target, server, LO_TT_IMMEDIATE, "/clock", "d", time() + simlat / 2.0f);
}

void RmmNode::setTestListener(TestListener *testListener) {
    this->testListener = testListener;
}

void RmmNode::handleClockReply(lo_address target, double t_remote) {
    // send nothing
    // calculate time difference
    double t2 = time();
    if (_recsynctime > 0) {
        if (timeOffset == 0) {
            timeOffset = (t_remote - (t2 + _recsynctime) / 2);
        }
        else
            timeOffset = timeOffset * 0.9 + (t_remote - (t2 + _recsynctime) / 2) * 0.1;
        
        printf("[%s-localh] TIME(%.3f,%.3f,%.3f))\n", this->nodeId.c_str(), _recsynctime, t_remote, t2-_recsynctime);
        printf("[%s-localh] TIME-offset adjusted to %.3f\n", this->nodeId.c_str(), timeOffset);
        _recsynctime = 0;
        onClockDiff(remote_time(), 0);
    }
    else {
        printf("[%s:ERROR] clock-reply recieved without being triggered\n", this->nodeId.c_str());
    }
    
}

void RmmNode::handleLatencyRequest(lo_address target) {
    lo_send_from(target, server, LO_TT_IMMEDIATE, "/latt", "d", time());
}

void RmmNode::handleSchedReceived(lo_address target, double t_send, const char * src, bool first) {
    // lo_send_from(target, server, LO_TT_IMMEDIATE, "/predrecv", "dsi", remote_time() - t_send, src, first?1:0);
    // printf("%.3f %s %d\n", remote_time() - t_send, src, first?1:0);
}

void RmmNode::handlePredResult(double lat, const char *src, int first) {
    if (testListener != NULL) {
        testListener->onPredRecved(lat, src, first > 0);
    }
}

void RmmNode::handleOutsideInput(double y) {
    onInput(y, 0);
}

void RmmNode::handleLatencyTest(lo_address target, double t) {
    if (_lattesttime > 0) {
        double rtt = remote_time() - _lattesttime;
        // printf("[%s-CLIENT] latency = %.3f\n", this->nodeId.c_str(), t - _lattesttime);
        double owt = t - _lattesttime;
        lat_oneway.push_back(owt);
        lat_rtt.push_back(rtt);
        
        _lattesttime = 0;
        double std = 0;
        double m = 0;
        int count = 0;
        for (int a = (int)lat_oneway.size() - 1; a >= 0 && a >= (int)lat_oneway.size() - 32; a--, count++) {
            m += lat_oneway[a];
        }
        m = m / (double)count;
        for (int a = (int)lat_oneway.size() - 1; a >= 0 && a >= (int)lat_oneway.size() - 32; a--) {
            std += (lat_oneway[a] - m) * (lat_oneway[a] - m);
        }
        std = std / (double)count;
        std = sqrt(std);
        onLatency(owt, rtt, std + std + m);
        lastlatencyupperbound = std + std + m;
    }
    else {
        printf("[%s-ERROR] latency test result got before asked\n", this->nodeId.c_str());
    }
}

void RmmNode::handleRedirection(lo_address target, const char * cid) {
    RmmConn conn;
    conn.nodeId = cid;
    const char *host = lo_address_get_hostname(target);
    const char *port = lo_address_get_port(target);
    conn.remoteIp = host;
    conn.remotePort = port;
    conn.status = RMM_ALIVE;
    conn.checkTime = time();
    printf("[%s-RELAY] REDIRECT[%s]-->[%s:%s]\n", this->nodeId.c_str(), cid, host, port);
    redirectlist.push_back(conn);
}

void RmmNode::handleRegistration(lo_address target, const char * cid) {
    const char *host = lo_address_get_hostname(target);
    const char *port = lo_address_get_port(target);
    addClient(host, port, cid);
    printf("[%s-RELAY] REGISTERED: %s => %s:%s\n", this->nodeId.c_str(), cid, host, port);
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

double RmmNode::latencyupper() {
    return lastlatencyupperbound > 0.01 ? lastlatencyupperbound : 0.01;
}


void RmmNode::join(const char *remoteIp, const char *remotePort) {
    printf("[%s:CLIENT-JOIN] joining...%s:%s\n", this->nodeId.c_str(), remoteIp, remotePort);
    this->connectedTo.remoteIp = remoteIp;
    this->connectedTo.remotePort = remotePort;
    clientInited = false;
    clientReset();
    
    int fd;
    struct ifreq ifr;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    
    ioctl(fd, SIOCGIFADDR, &ifr);
    
    ::close(fd);
    
    /* display result */
    // printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    std::string localaddr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    
    int r = lo_send_from(client, server, LO_TT_IMMEDIATE, "/join", "s", this->nodeId.c_str());
    if (r < 0)
        printf("[%s:CLIENT-JOIN] Error sending initial message.\n", this->nodeId.c_str());
    clientInited = true;
    connectedTo.status = RMM_TOJOIN;
    _timeouttime = time();
}

void RmmNode::clientReset() {
    if (clientInited == true) {
        lo_address_free(client);
    }
    client = lo_address_new_with_proto(useTcp?LO_TCP:LO_UDP, this->connectedTo.remoteIp.c_str(), this->connectedTo.remotePort.c_str());
    if (!client) {
        printf("[%s:CLIENT] Error creating destination address.\n", this->nodeId.c_str());
    }
}

void RmmNode::poll() {
    double t0 = time();
    double t1 = remote_time();
    
    // printf("t=%f (next-cmd %.3f, next-ev %.3f)\n", t0, t_schedcmd, event.time);
    if (t_schedcmd > 0 && t0 > t_schedcmd) {
        exec_rmmcode(code_schedcmd);
        if (t0 > t_schedcmd) {
            t_schedcmd = 0;
        }
    }
    if (event.time > 0 && t1 > event.time) {
        if (event.time - _last_exec_time > 0.2) {
            printf("%.3f %.3f\n", t1, event.time);
            exec_event(event);
            
            if (testListener != NULL) {
                testListener->onExec(t1, event.time, _schedtracetime, _schedrecvtime);
            }
            
            if (t1 > event.time) {
                event.time = 0;
            }
            _last_exec_time = t1;
        }
        
    }
    else {
    }
    
    if (t0 > latt) {
        latt = ((int)(t0 + t0)) / 2.0 + 0.5;
        if (connectedTo.status == RMM_ALIVE)
            sendLatencyTest();
    }
    
    if (needCleanUpConnectionList) {
        needCleanUpConnectionList = false;
        std::vector<RmmConn>::iterator iter = connections.begin();
        for (; iter != connections.end(); ++iter) {
            if (iter->status == RMM_OUTOFSYNC) {
                printf("[%s-localh] deconnect unresponsed client [%s]\n", this->nodeId.c_str(), iter->nodeId.c_str());
                connections.erase(iter);
                break;
            }
        }
    }
    
    if (needCleanUpRedirectList) {
        needCleanUpRedirectList = false;
        std::vector<RmmConn>::iterator iter = redirectlist.begin();
        for (; iter != redirectlist.end(); ++iter) {
            if (iter->status == RMM_OUTOFSYNC) {
                printf("[%s-localh] deconnect unresponsed redirection route [%s]\n", this->nodeId.c_str(), iter->nodeId.c_str());
                redirectlist.erase(iter);
                break;
            }
        }
    }
}

void RmmNode::schedule(double srctime, double t, int cmd, int arg) {
    if (srctime > _schedtracetime) {
        event.time = t;
        event.cmd = cmd;
        event.arg = arg;
        printf("\t%.3f %.3f\n", t, time());
        _schedtracetime = srctime;
        _schedrecvtime = remote_time();
    }
}

void RmmNode::schedule(lo_address target, double srctime, double t, int cmd, int arg, const char * src) {
    if (srctime > _schedtracetime) {
        event.time = t;
        event.cmd = cmd;
        event.arg = arg;
        _schedtracetime = srctime;
        _schedrecvtime = remote_time();
        printf("\trecv=%.3f pred=%.3f rmt=%.3f\n", time(), t, _schedrecvtime);
        handleSchedReceived(target, srctime, src, true);
    }
    else {
        handleSchedReceived(target, srctime, src, false);
    }
}


void RmmNode::handlePred(double t_send, double t_pred, int cmd, int arg) {
    double curr_time = remote_time();
    if (curr_time - t_send > t_pred - t_send) {
        
    }
}

void RmmNode::handleChat(lo_address target, char * content) {
    const char *host = lo_address_get_hostname(target);
    const char *port = lo_address_get_port(target);
    printf("[%s:%s SAYS] %s\n", host, port, content);
    onChat(content);
}

void RmmNode::handleJoinSuccess(lo_address target, const char * sid) {
    if (connectedTo.status == RMM_TOJOIN) {
        connectedTo.status = RMM_CONNECTED;
        connectedTo.nodeId = sid;
        // start clock sync one second later
        delayedcommand(time() + 1, RMM_CLOCKSYNC);
        printf("[%s:CLIENT] join sucessful\n", this->nodeId.c_str());
    }
    else {
        printf("[%s:ERROR] join-successs received without asked\n", this->nodeId.c_str());
    }
}

void RmmNode::setRelayDelay(float delay) {
    this->relaydelay = delay;
}

void RmmNode::handlePrintStatus() {
    printf("----------------------------------------------\n");
    printf("|%-10s|%-24s|%-8s|\n", " ID", "  Temp Address", " status");
    printf("----------------------------------------------\n");
    for (int a = 0; a < connections.size(); ++a) {
        printf("| %-9s|%17s:%-6s| %-7s|\n", connections[a].nodeId.c_str(), connections[a].remoteIp.c_str(), connections[a].remotePort.c_str(), connections[a].status==RMM_OUTOFSYNC?" -- ":"active");
    }
    printf("----------------------------------------------\n");
}

void RmmNode::sendKeepAlive() {
    _recsynctime = time();
    if (clientInited) {
        lo_send_from(client, server, LO_TT_IMMEDIATE, "/syncreq", "d", _recsynctime);
    }
}

void RmmNode::setDebug(bool debug) {
    _debug = debug;
}

void RmmNode::exec_rmmcode(RmmCodes code) {
    switch (code) {
        case RMM_CLOCKSYNC:
            sendClockRequest();
            if (clocksynccount > 0) {
                clocksynccount --;
                delayedcommand(time() + 0.5, RMM_CLOCKSYNC);
            }
            else {
                clocksynccount = 3;
                delayedcommand(time() + 20, RMM_CLOCKSYNC);
                connectedTo.status = RMM_ALIVE;
            }
            break;
            
        case RMM_KEEPALIVE:
            sendKeepAlive();
            break;
            
        case RMM_CLOCKREP:
            double tdf = (rand()%100)/1000.0 + 0.02;
            handleClockRequest(tempaddr, tdf);
            break;
            
    }
}

void RmmNode::exec_event(RmmEvent &ev) {
    this->onEvent(ev.cmd, ev.arg);
}

lo_server RmmNode::getserverptr() {
    return server;
}

void RmmNode::setForcedTimeOffset(double toff) {
    this->forcedoffset = toff;
}

void RmmNode::setForceNoClocksync(bool ncs) {
    forceNoClockSync = ncs;
}


void RmmNode::delayedcommand(double t_ms, RmmCodes code) {
    t_schedcmd = t_ms;
    code_schedcmd = code;
}

bool RmmNode::isUsingTcp() {
    return useTcp;
}

RmmConn RmmNode::convertMapping(const char * cid) {
    std::string str = cid;
    for (int a = 0; a < connections.size(); ++a) {
        if (str == connections[a].nodeId) {
            return connections[a];
        }
        else {
        }
    }
    RmmConn empty; empty.status = RMM_OUTOFSYNC;
    return empty;
}

bool RmmNode::redirect(lo_address target, lo_message m, const char * path) {
    std::string host = lo_address_get_hostname(target);
    std::string port = lo_address_get_port(target);
    for (int a = 0; a < redirectlist.size(); ++a) {
        if (redirectlist[a].remoteIp == host && redirectlist[a].remotePort == port) {
            RmmConn conn = convertMapping(redirectlist[a].nodeId.c_str());
            lo_address fwdaddr = lo_address_new(conn.remoteIp.c_str(), conn.remotePort.c_str());
            if (relaydelay != 0) {
                double t = time();
                // artificial delay
                while (time() < t + relaydelay / 1000.0) {
                    usleep(1000);
                }
            }
            lo_send_message_from(fwdaddr, server, path, m);
            lo_address_free(fwdaddr);
            return true;
        }
    }
    return false;
}

std::string RmmNode::getNodeId() {
    return nodeId;
}

////////////////////
///////// For C-style callback functions
////////////////////

/* catch any incoming messages, display them, and send them
 * back. */
int echo_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data)
{
    int i;
    RmmNode * node = (RmmNode *)(user_data);
    lo_message m = (lo_message)data;
    lo_address a = lo_message_get_source(m);
    node->tempaddr = a;
    // lo_server  s = node->getserverptr();
    // const char *host = lo_address_get_hostname(a);
    // const char *port = lo_address_get_port(a);
    
    std::string pathstr = path;
    
    if (pathstr == "/status") {
        node->handlePrintStatus();
    }
    else if (node->redirect(a, m, path)) {
        node->handleAlive(a);
        return 1;
    }
    // see if redirection needed
    
    
    if (!a) {
        printf("[FATAL] Cannot parse message address\n");
        return 0;
    }
    if (pathstr == "/sched") {
        double t_send = *(double*)argv[0];
        double t_pred = *(double*)argv[1];
        int cmd = *(int*)argv[2];
        int arg = *(int*)argv[3];
        std::string src = (char *)argv[4];
        // node->lastaddr = a;
        node->schedule(a, t_send, t_pred, cmd, arg, src.c_str());
        
    }
    else if (pathstr == "/pred") {
        double t_send = *(double*)argv[0];
        double t_pred = *(double*)argv[1];
        int cmd = *(int*)argv[2];
        int arg = *(int*)argv[3];
        std::string src = (char *)argv[4];
        // node->lastaddr = a;
        node->handlePred(t_send, t_pred, cmd, arg);
        // printf("   --- sched received from %s\n", lo_address_get_hostname(a));
    }
    else if (pathstr == "/syncreq") {
        node->handleClockRequest(a);
    }
    else if (pathstr == "/chat") {
        node->handleChat(a, (char*)argv[0]);
    }
    else if (pathstr == "/clock") {
        node->handleClockReply(a, *(double*)argv[0]);
    }
    else if (pathstr == "/latt") {
        node->handleLatencyTest(a, *(double*)argv[0]);
    }
    else if (pathstr == "/join") {
        node->handleJoin(a, (char *)argv[0]);
    }
    else if (pathstr == "/joined") {
        node->handleJoinSuccess(a, (char *)argv[0]);
    }
    else if (pathstr == "/lattreq") {
        node->handleLatencyRequest(a);
    }
    else if (pathstr == "/register") {
        node->handleRegistration(a, (char *)argv[0]);
    }
    else if (pathstr == "/status") {
        
    }
    else if (pathstr == "/redirect") {
        node->handleRedirection(a, (char *)argv[0]);
    }
    else if (pathstr == "/predrecv") {
        node->handlePredResult(*(double*)argv[0], (char*)argv[1], *(int*)argv[2]);
    }
    else if (pathstr == "/rgb") {
        node->handleOutsideInput(*(double*)argv[0]);
        printf("  I got %.3f\n", *(double*)argv[0]);
    }
    else if (pathstr == "/fwd") {
        
         RmmConn converted = node->convertMapping((char*)argv[1]);
        if (converted.status == RMM_OUTOFSYNC) {
            
            std::string ipport = (char*)argv[1];
            size_t idx = ipport.find_first_of(":");
            if (idx ==std::string::npos) {
                printf("[%s-OSC] fwd failed with [%s]\n", node->getNodeId().c_str(), (char*)argv[1]);
                return -1;
            }
            converted.remoteIp = ipport.substr(0, idx);
            converted.remotePort = ipport.substr(idx + 1);
            printf("[%s-OSC] use public address: %s:%s\n", node->getNodeId().c_str(), converted.remoteIp.c_str(), converted.remotePort.c_str());
        }
        std::string path = (char*)argv[0];
        std::string dstIp = converted.remoteIp;
        std::string dstPort = converted.remotePort;
        const char * types = lo_message_get_types(m);
        
        lo_message msg = lo_message_new();
        size_t len = strlen(types);
        for (int a = 2; a < len; ++a) {
            if (types[a] == 'd') {
                lo_message_add_double(msg, *((double*)argv[a]));
            }
            else if (types[a] == 'f') {
                lo_message_add_float(msg, *((float*)argv[a]));
            }
            else if (types[a] == 's') {
                if (path == "/sched" && a == 6) {
                    lo_message_add_string(msg, node->getNodeId().c_str());
                }
                else {
                    lo_message_add_string(msg, (char*)argv[a]);
                }
            }
            else if (types[a] == 'i') {
                lo_message_add_int32(msg, *((int*)argv[a]));
            }
        }
        
        
        lo_address fwdaddr = lo_address_new_with_proto(node->isUsingTcp()?LO_TCP:LO_UDP, dstIp.c_str(), dstPort.c_str());
        
        int ret = lo_send_message_from(fwdaddr, node->getserverptr(), path.c_str(), msg);
        if (ret >= 0) {
            printf("[%s-F>>>%s:%s] (%s)(", node->getNodeId().c_str(), dstIp.c_str(), dstPort.c_str(), path.c_str());
        }
        else {
            printf("[%s-FAILED F>>>%s:%s] (%s)(", node->getNodeId().c_str(), dstIp.c_str(), dstPort.c_str(), path.c_str());
        }
        
        for (i = 2; i < argc; i++) {
            lo_arg_pp((lo_type)types[i], argv[i]);
            printf(", ");
        }
        lo_address_free(fwdaddr);
        printf(")\n");
    }
    else {
        printf("[UNKNOWN:%s] message(", path);
        for (i = 0; i < argc; i++) {
            // printf(" arg %d '%c' ", i, types[i]);
            lo_arg_pp((lo_type)types[i], argv[i]);
            printf(", ");
        }
        printf("\b\b)");
        printf("\n");
    }
    
    node->handleAlive(a);
    
    return 0;
}


int quit_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data)
{
    printf("quitting\n\n");
    fflush(stdout);
    
    return 0;
}

