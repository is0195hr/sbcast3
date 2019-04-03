#ifndef SBCAST_H_
#define SBCAST_H_




#include "agent.h"
#include "classifier-port.h"
#include "ip.h"
#include "packet.h"
#include "cmu-trace.h"




#define CURRENT_TIME Schedular::instance().clock()
#define HDR_BEACON(p) (hdr_beacon *) hdr_beacon::access(p)

// Beacon packet header
struct hdr_beacon {
    nsaddr_t addr_;
    u_int8_t seq_num_;
    int pktnum_;
    int pkttype_;
    int pkt1_,pkt2_,pkt3_,pkt4_,pkt5_;
    int codenum_;
    int codevc_;
    int hop_count_;
    int encode_count_;
    inline nsaddr_t & addr() { return addr_; }
    inline u_int8_t & seq_num() { return seq_num_; }
    static int offset_;
    inline static int & offset() { return offset_; }
    inline static hdr_beacon * access(const Packet *p) {
        return (hdr_beacon*) p->access(offset_);
    }
};

// Simple Broadcast Agent
class SBAgent;


//Broadcast Timer class
class BcastTimer : public TimerHandler {
protected:
    SBAgent *agent_;
    virtual void expire(Event *e);
public:
    BcastTimer(SBAgent *agent):TimerHandler(),agent_(agent) { }
};


class SBAgent : public Agent {
protected:
    friend class BcastTimer;
private:
    BcastTimer btimer_;
    PortClassifier *dmux_;
    Trace *logtarget_;
    nsaddr_t my_addr_;
    int seq_num_;
public:
    SBAgent();
    int command(int,const char*const*);
    void recv(Packet *,Handler *);
    void sendBeacon();
    void sendRequest();
    void resetBcastTimer();
    int createCodepacket2(int,int);
    int createCodepacket3(int,int,int);
    int createCodepacket4(int,int,int,int);
    int createCodepacket5(int,int,int,int,int);
    inline nsaddr_t & my_addr() { return my_addr_;}
};
#endif /* SBCAST_H_ */