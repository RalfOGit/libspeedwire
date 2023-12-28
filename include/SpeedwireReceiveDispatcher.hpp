#ifndef __LIBSPEEDWIRE_SPEEDWIRERECEIVEDISPATCHER_HPP__
#define __LIBSPEEDWIRE_SPEEDWIRERECEIVEDISPATCHER_HPP__

#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireSocket.hpp>

namespace libspeedwire {

    /**
     * Interface to be implemented by any packet receiver.
     */
    class SpeedwirePacketReceiverBase {
    public:
        uint16_t protocolID;        //!< Protocol ID that the receiver is configured to receive

        /**
         * Constructor - must be overridden; it initialzes protocolID to 0x0000.
         * @param host Reference to LocalHost instance.
         */
        SpeedwirePacketReceiverBase(LocalHost& host) : protocolID(0x0000) {}
        virtual ~SpeedwirePacketReceiverBase(void) {}

        /**
         * Virtual receive method - must be overriden.
         * @param packet Reference to a packet instance that was received from the socket.
         * @param src Reference to a socket address with the ip address and port of the packet sender.
         */
        virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
    };


    /**
     * Interface to beimplemented by emeter packet receivers.
     */
    class EmeterPacketReceiverBase : public SpeedwirePacketReceiverBase {
    public:

        /**
         * Constructor - it initialzes protocolID to SpeedwireHeader::sma_emeter_protocol_id.
         * @param host Reference to LocalHost instance.
         */
        EmeterPacketReceiverBase(LocalHost& host) : SpeedwirePacketReceiverBase(host) {
            protocolID = SpeedwireData2Packet::sma_emeter_protocol_id;
        }

        /**
         * Virtual receive method - must be overriden.
         * @param packet Reference to a packet instance that was received from the socket.
         * @param src Reference to a socket address with the ip address and port of the packet sender.
         */
        virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
    };


    /**
     * Interface to beimplemented by inverter packet receivers.
     */
    class InverterPacketReceiverBase : public SpeedwirePacketReceiverBase {
    public:

        /**
         * Constructor - it initialzes protocolID to SpeedwireHeader::sma_inverter_protocol_id.
         * @param host Reference to LocalHost instance.
         */
        InverterPacketReceiverBase(LocalHost& host) : SpeedwirePacketReceiverBase(host) {
            protocolID = SpeedwireData2Packet::sma_inverter_protocol_id;
        }

        /**
         * Virtual receive method - must be overriden.
         * @param packet Reference to a packet instance that was received from the socket.
         * @param src Reference to a socket address with the ip address and port of the packet sender.
         */
        virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
    };


    /**
     * Interface to beimplemented by discovery packet receivers.
     */
    class DiscoveryPacketReceiverBase : public SpeedwirePacketReceiverBase {
    public:

        /**
         * Constructor - it initialzes protocolID to 0x0000.
         * @param host Reference to LocalHost instance.
         */
        DiscoveryPacketReceiverBase(LocalHost& host) : SpeedwirePacketReceiverBase(host) {
            protocolID = 0x0000;
        }

        /**
         * Virtual receive method - must be overriden.
         * @param packet Reference to a packet instance that was received from the socket.
         * @param src Reference to a socket address with the ip address and port of the packet sender.
         */
        virtual void receive(SpeedwireHeader& packet, struct sockaddr& src) = 0;
    };


    /**
     * Class implementing a receiver and dispatcher for speedwire packets.
     * Classes interested in receiving speedwire packets can register themselves to this class. Calls to
     * the dispatch method poll all given sockets, receive packet data, check its validity and dispatches
     * the packet to any corresponding registered receiver.
     */
    class SpeedwireReceiveDispatcher {
    protected:
        LocalHost& localhost;
        std::vector<SpeedwirePacketReceiverBase*> receivers;
        std::vector<struct pollfd> pollfds;

    public:
        SpeedwireReceiveDispatcher(LocalHost& localhost);
        ~SpeedwireReceiveDispatcher(void);

        int  dispatch(const std::vector<SpeedwireSocket>& sockets, const int poll_timeout_in_ms);

        void registerReceiver(SpeedwirePacketReceiverBase& receiver);
        void registerReceiver(EmeterPacketReceiverBase& receiver);
        void registerReceiver(InverterPacketReceiverBase& receiver);
        void registerReceiver(DiscoveryPacketReceiverBase& receiver);
    };

}   // namespace libspeedwire

#endif
