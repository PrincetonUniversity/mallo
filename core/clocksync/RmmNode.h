//
//  RmmNode.h
//  clocksync
//
//  Created by Zeyu Jin on 1/6/15.
//  Copyright (c) 2015 Zeyu Jin. All rights reserved.
//

#ifndef __clocksync__RmmNode__
#define __clocksync__RmmNode__

#include <stdio.h>
#include <string>
#include <vector>

#include <lo/lo.h>

enum RmmCodes {
    RMM_CLOCKSYNC = 0x1,
    RMM_KEEPALIVE = 0x2,
    RMM_CLOCKREP = 0x10
};

enum RmmStats {
    RMM_OFFLINE, RMM_TOJOIN, RMM_OUTOFSYNC, RMM_ALIVE, RMM_CONNECTED
};

struct RmmConn {
    std::string nodeId;
    std::string remoteIp;
    std::string remotePort;
    RmmStats status;
    lo_address  remote;
    double checkTime = 0;
};

struct RmmEvent {
    int cmd;
    int arg;
    double time = 0;
};

class RmmListener {
public:
    virtual void onEvent(int cmd, int arg) = 0;
    virtual void onChat(const char * content) = 0;
    virtual void onLatency(double ol, double rtl, double upper) = 0;
    virtual void onClockDiff(double t, double tdiff) = 0;
    virtual void onPrediction(double t, double t0) = 0;
    virtual void onInput(double y, double z) = 0;
};

class TestListener {
public:
    virtual void onExec(double t, double t_sched_to, double t_sched_send, double t_pred_recv)= 0;
    virtual void onPredRecved(double lat, const char * src, bool firstarrival) = 0;
};

class RmmNode : public RmmListener {
    bool inited = false;
    bool useTcp = false;
    bool clientInited = false;
    bool synced = false;
    bool forceNoClockSync = false;
    RmmConn connectedTo;
    
    std::string localPort;
    
    double lastlatencyupperbound = 0;
    
    bool needCleanUpConnectionList = false;
    bool needCleanUpRedirectList = false;
    
    TestListener * testListener = NULL;
    
protected:
    
    std::vector<double> lat_oneway;
    std::vector<double> lat_rtt;
    std::vector<RmmConn> connections;
    std::vector<RmmEvent> events;
    
    std::vector<RmmConn> relayConnections;
    std::vector<RmmConn> redirectlist;;
    
    lo_server server;
    lo_server_thread st;
    
    lo_address client;
    
    lo_timetag t0;
    double timeOffset = 0;
    double forcedoffset = 0;
    
protected:
    
    std::string nodeId;
    bool _debug = false;
    
protected:
    int clocksynccount = 5;
    double _recsynctime = 0;
    double _lattesttime = 0;
    double _timeouttime = 0;
    double _schedtracetime = 0;
    double _schedrecvtime = 0;
    double latt = 0;
    double _latestimate = 0;
    double _last_exec_time = 0;
    
public:
    RmmNode(const char * port, const char * nodeId);
    void start(bool useTcp);
    void requestRedirection(const char *host, const char *port, const char * targetHost);
    void requestRegistration(const char *host, const char *port);
    void join(const char * remoteIp, const char * remotePort);
    void restart(const char * port, bool useTcp);
    void poll();
    void schedule(double srctime, double t, int cmd, int arg);
    void schedule(lo_address target, double srctime, double t, int cmd, int arg, const char * src);
    double time();
    double remote_time();
    double remote_time(double t);
    double latencyupper();
    void alignTime(lo_timetag tag);
    
    RmmConn convertMapping(const char * cid);
    
    void exec_rmmcode(RmmCodes code);
    void exec_event(RmmEvent &ev);
    
    void addRelayConnection(const char * ip, const char * port);
    void addClient(const char * ip, const char * port, const char * cid);
    void setTestListener(TestListener * testListener);
    
    // for debug
    void setForceNoClocksync(bool ncs);
    void setForcedTimeOffset(double toff);
    void setDebug(bool debug);
    bool isDebug();
    std::string getNodeId();
    lo_address tempaddr;
    
    void multisend(lo_address target, lo_message &msg, const char * path);
    bool isUsingTcp();
    
    bool redirect(lo_address target, lo_message m, const char * path);
    
    void changeTargetName(const char * name);
    
protected:
    void close();
    void clientReset();
    
public: // sending messages
    
    void sendSched(double t_ms, int midiCode, int content);
    void sendPred(double t_ms, int midiCode, int content);
    void sendClockRequest();
    void sendKeepAlive();
    void sendLatencyTest();
    
protected:
    double t_schedcmd; RmmCodes code_schedcmd;
    RmmEvent event;
    
    
public:
    // SERVER actions
    void delayedcommand(double t_ms, RmmCodes code);
    void handleClockRequest(lo_address target);
    void handleClockRequest(lo_address target, double latt);
    void handleJoin(lo_address target, const char * cid);
    void handleAlive(lo_address target);
    void handleAlive();
    void handleLatencyRequest(lo_address target);
    void handleForward(lo_address target);
    void handlePred(double t_send, double t_pred, int cmd, int arg);
    void handleSchedReceived(lo_address target, double t_send, const char * src, bool isFirst);
    void handlePredResult(double lat, const char * src, int first);
    
    // CLIENT ACTIONS
    void handleClockReply(lo_address target, double time);
    void handleChat(lo_address target, char * content);
    void handleJoinSuccess(lo_address target, const char * sid);
    virtual void handleLatencyTest(lo_address target, double t);
    virtual void handleOutsideInput(double y);
    
    // FOR RELAY
    double relaydelay = 0;
    void rtt2relay();
    lo_address lastaddr;
    void handleRedirection(lo_address target, const char * cid);
    void handleRegistration(lo_address target, const char * cid);
    void handlePrintStatus();
    void setRelayDelay(float delay);
    
public:  // user messages
    void sendChat(const char * content);
    
public:
    lo_server getserverptr();
};

#endif /* defined(__clocksync__RmmNode__) */
