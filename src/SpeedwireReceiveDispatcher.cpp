#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#define poll(a, b, c)  WSAPoll((a), (b), (c))
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#endif

#include <AddressConversion.hpp>
#include <Logger.hpp>
#include <SpeedwireReceiveDispatcher.hpp>

static Logger logger("SpeedwireReceiveDispatcher");


/**
 *  Constructor and destructor
 */
SpeedwireReceiveDispatcher::SpeedwireReceiveDispatcher(LocalHost& _localhost)
  : localhost(_localhost) {}

SpeedwireReceiveDispatcher::~SpeedwireReceiveDispatcher(void) {
    receivers.clear();
    pollfds.clear();
}


/**
 *  Dispatch method - polls on all given sockets and dispatches to registered receivers
 */
int  SpeedwireReceiveDispatcher::dispatch(const std::vector<SpeedwireSocket>& sockets, const int poll_timeout_in_ms) {
    int npackets = 0;
    unsigned char udp_packet[2048];

    // make sure the backing array of the vector is big enough (yes, it is contiguous memory)
    if (pollfds.size() < sockets.size()) {
        pollfds.resize(sockets.size());
    }

    // prepare the pollfd structure
    for (int j = 0; j < sockets.size(); ++j) {
        pollfds[j].fd = sockets[j].getSocketFd();
        pollfds[j].events = POLLIN;
        pollfds[j].revents = 0;
    }

    // wait for a packet on the configured socket
    if (poll(&pollfds[0], (unsigned)sockets.size(), poll_timeout_in_ms) < 0) {
        perror("poll failure");
        return -1;
    }

    // determine if the socket received a packet
    for (int j = 0; j < sockets.size(); ++j) {
        const SpeedwireSocket& socket = sockets[j];

        if ((pollfds[j].revents & POLLIN) != 0) {

            // read packet data
            int nbytes = -1;
            struct sockaddr src;
            if (socket.isIpv4()) {
                nbytes = socket.recvfrom(udp_packet, sizeof(udp_packet), AddressConversion::toSockAddrIn(src));
            }
            else if (socket.isIpv6()) {
                nbytes = socket.recvfrom(udp_packet, sizeof(udp_packet), AddressConversion::toSockAddrIn6(src));
            }

            // check if it is an sma speedwire packet
            SpeedwireHeader speedwire_packet(udp_packet, nbytes);
            bool valid_speedwire_packet = speedwire_packet.checkHeader();
            if (valid_speedwire_packet) {
                uint32_t group      = speedwire_packet.getGroup();
                uint16_t length     = speedwire_packet.getLength();
                uint16_t protocolID = speedwire_packet.getProtocolID();
                int      offset     = speedwire_packet.getPayloadOffset();

                bool valid_emeter_packet = false;
                bool valid_inverter_packet = false;

                // check if it is an sma emeter packet
                if (speedwire_packet.isEmeterProtocolID()) {
                    SpeedwireEmeterProtocol emeter(speedwire_packet);
                    uint16_t susyid = emeter.getSusyID();
                    uint32_t serial = emeter.getSerialNumber();
                    uint32_t timer  = emeter.getTime();
                    logger.print(LogLevel::LOG_INFO_2, "RECEIVED EMETER PACKET  time 0x%016llx\n", timer);
                    valid_emeter_packet = true;
                    ++npackets;
                }
                // check if it is an sma inverter packet
                else if (speedwire_packet.isInverterProtocolID()) {
                    uint8_t longwords = speedwire_packet.getLongWords();

                    // a few quick sanity checks
                    if ((length + (size_t)20) > sizeof(udp_packet)) {    // packet length - starting to count from the byte following protocolID, # of long words and control byte, i.e. with byte #20
                        logger.print(LogLevel::LOG_ERROR, "length field %u and buff_size %u mismatch\n", length, (unsigned)sizeof(udp_packet));
                        return -1;
                    }
                    if (length < (8 + 8 + 6)) {                         // up to and including packetID
                        logger.print(LogLevel::LOG_ERROR, "length field %u too small to hold inverter packet (8 + 8 + 6)\n", length);
                        return -1;
                    }
                    if ((longwords != (length / sizeof(uint32_t)))) {
                        logger.print(LogLevel::LOG_ERROR, "length field %u and long words %u mismatch\n", length, longwords);
                        return -1;
                    }

                    SpeedwireInverterProtocol inverter(speedwire_packet);
                    logger.print(LogLevel::LOG_INFO_2, "RECEIVED INVERTER PACKET  time 0x%016llx\n", LocalHost::getUnixEpochTimeInMs());
                    valid_inverter_packet = true;
                    ++npackets;
                }
                else {
                    logger.print(LogLevel::LOG_INFO_0, "RECEIVED UNKNOWN PROTOCOL 0x%04x time 0x%016llx\n", protocolID, LocalHost::getUnixEpochTimeInMs());
                }

                // pass it to the relevant registered packet consumers
                for (auto& receiver : receivers) {
                    switch (receiver->protocolID) {
                    case 0x0000:
                        receiver->receive(speedwire_packet, src);
                        break;
                    case SpeedwireHeader::sma_emeter_protocol_id:
                        if (valid_emeter_packet == true) {
                            receiver->receive(speedwire_packet, src);
                        }
                        break;
                    case SpeedwireHeader::sma_inverter_protocol_id:
                        if (valid_inverter_packet == true) {
                            receiver->receive(speedwire_packet, src);
                        }
                        break;
                    }
                }
            }
        }
    }
    return npackets;
}


/**
 *  Registration methods for receivers
 */
void SpeedwireReceiveDispatcher::registerReceiver(SpeedwirePacketReceiverBase *receiver) {
    receiver->protocolID = 0x0000;
    receivers.push_back(receiver);
}

void SpeedwireReceiveDispatcher::registerReceiver(EmeterPacketReceiverBase *receiver) {
    receiver->protocolID = SpeedwireHeader::sma_emeter_protocol_id;
    receivers.push_back(receiver);
}

void SpeedwireReceiveDispatcher::registerReceiver(InverterPacketReceiverBase *receiver) {
    receiver->protocolID = SpeedwireHeader::sma_inverter_protocol_id;
    receivers.push_back(receiver);
}
