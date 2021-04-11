#ifndef __SPEEDWIRERECEIVEDISPATCHER_HPP__
#define __SPEEDWIRERECEIVEDISPATCHER_HPP__

#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireSocket.hpp>


// Base class for speedwire packet receivers
class SpeedwirePacketReceiverBase {
public:
    uint16_t protocolID;
    SpeedwirePacketReceiverBase(LocalHost& host) : protocolID(0x0000) {}
    virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
};

// Base class for sma emeter packets
class EmeterPacketReceiverBase : public SpeedwirePacketReceiverBase {
public:
    EmeterPacketReceiverBase(LocalHost& host) : SpeedwirePacketReceiverBase(host) {
        protocolID = SpeedwireHeader::sma_emeter_protocol_id;
    }
    virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
};

// Base class for sma inverter packets
class InverterPacketReceiverBase : public SpeedwirePacketReceiverBase {
public:
    InverterPacketReceiverBase(LocalHost& host) : SpeedwirePacketReceiverBase(host) {
        protocolID = SpeedwireHeader::sma_inverter_protocol_id;
    }
    virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
};



class SpeedwireReceiveDispatcher {
protected:
    LocalHost& localhost;
    std::vector<SpeedwirePacketReceiverBase*> receivers;
    std::vector<struct pollfd> pollfds;

public:
    SpeedwireReceiveDispatcher(LocalHost& localhost);
    ~SpeedwireReceiveDispatcher(void);

    int  dispatch(const std::vector<SpeedwireSocket>& sockets, const int poll_timeout_in_ms);

    void registerReceiver(SpeedwirePacketReceiverBase *receiver);
    void registerReceiver(EmeterPacketReceiverBase    *receiver);
    void registerReceiver(InverterPacketReceiverBase  *receiver);
};

#endif
