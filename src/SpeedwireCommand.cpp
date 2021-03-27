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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>
#include <SpeedwireCommand.hpp>


const uint16_t SpeedwireCommand::local_susy_id = 0x007d;
const uint32_t SpeedwireCommand::local_serial_id = 0x3a28be42;


SpeedwireCommand::SpeedwireCommand(const LocalHost &_localhost, const std::vector<SpeedwireInfo> &_devices) :
    localhost(_localhost),
    devices(_devices) {
    // loop across all speedwire devices
    for (auto& device : devices) {
        // check if there is already a map entry for the interface ip address
        if (socket_map.find(device.interface_ip_address) == socket_map.end()) {
            // create and open a socket for the interface
            SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getRecvSocket(SpeedwireSocketFactory::UNICAST, device.interface_ip_address);
            if (socket.getSocketFd() >= 0) {
                // add it to the map
                socket_map[device.interface_ip_address] = (SocketIndex)sockets.size();
                sockets.push_back(socket);
            }
            else {
                socket_map[device.interface_ip_address] = -1;
            }
        }
    }
    packet_id = 0x8001;
}

SpeedwireCommand::~SpeedwireCommand(void) {
    sockets.clear();
    socket_map.clear();
}


/*
 *  Send login packet to peer, wait for response and check result
 */
int32_t SpeedwireCommand::login(const SpeedwireInfo& peer, const bool user, const char* password) {
    // Request  534d4100000402a000000001003a0010 60650ea0 7a01842a71b30001 7d0042be283a0001 000000000280 0c04fdff 07000000 84030000 00d8e85f 00000000 c1c1c1c18888888888888888 00000000   => login command = 0xfffd040c, first = 0x00000007 (user 7, installer a), last = 0x00000384 (hier timeout), time = 0x5fdf9ae8, 0x00000000, pw 12 bytes
    // Response 534d4100000402a000000001002e0010 60650be0 7d0042be283a0001 7a01842a71b30001 000000000280 0d04fdff 07000000 84030000 00d8e85f 00000000 00000000 => login OK
    // Response 534d4100000402a000000001002e0010 60650be0 7d0042be283a0001 7a01842a71b30001 000100000280 0d04fdff 07000000 84030000 fddbe85f 00000000 00000000 => login INVALID PASSWORD
    // command  0xfffd040c => 0x400 set?  0x00c bytecount=12?
    // assemble unicast device login packet
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4 + 4 + 4 + 12 + 4];
    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer)-20, SpeedwireHeader::sma_inverter_protocol_id);
    request_header.setControl(0xa0);
    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(peer.susyID);
    request.setDstSerialNumber(peer.serialNumber);
    request.setDstControl(0x0100);
    request.setSrcSusyID(local_susy_id);
    request.setSrcSerialNumber(local_serial_id);
    request.setSrcControl(0x0100);
    request.setErrorCode(0);
    request.setFragmentID(0);
    request.setPacketID(packet_id);
    request.setCommandID(0xfffd040c);
    request.setFirstRegisterID((user?0x00000007:0x0000000a));    // user: 0x7  installer: 0xa
    request.setLastRegisterID(0x00000384);     // timeout
    request.setDataUint32(0, (uint32_t)time(NULL));
    request.setDataUint32(4, 0x00000000);
    uint8_t encoded_password[12];
    memset(encoded_password, (user ? 0x88 : 0xBB), sizeof(encoded_password));
    for (int i = 0; password[i] != '\0' && i < sizeof(encoded_password); ++i) {
        encoded_password[i] = password[i] + (user ? 0x88 : 0xBB);
    }
    request.setDataUint8Array(8, encoded_password, sizeof(encoded_password));
    request.setTrailer(20);

    // send login request packet to peer
    SocketIndex socket_index = socket_map[peer.interface_ip_address];
    if (socket_index < 0) {
        perror("invalid socket_index");
        return -1;
    }
    SpeedwireSocket& socket = sockets[socket_index];
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), peer.peer_ip_address);

    // receive login reply packet from peer
    unsigned char reply_buffer[1500];
    struct sockaddr recvfrom;
    int  nrecv = -1;
    bool valid = false;
    while (valid == false && nrecv != 0) {  // there can be any number of emeter packets in front of the reply packet
        nrecv = recvReply(peer, reply_buffer, sizeof(reply_buffer), 1000, recvfrom);
        valid = checkReply(peer, packet_id, recvfrom, reply_buffer, nrecv);
    }
    packet_id = (packet_id + 1) | 0x8000;
    if (valid == false) {
        perror("invalid login reply data");
        return -1;
    }

    // check error code
    SpeedwireHeader reply_header(reply_buffer, sizeof(reply_buffer));
    SpeedwireInverterProtocol reply(reply_header);
    uint16_t result = reply.getErrorCode();
    if (result != 0x0000) {
        if (result == 0x0100) {
            perror("invalid password");
        } else {
            perror("login failure");
        }
        return -1;
    }

    return 0;
}

int32_t SpeedwireCommand::logoff(const SpeedwireInfo& peer) {
    // Request 534d4100000402a00000000100220010 606508a0 ffffffffffff0003 7d0052be283a0003 000000000280 0e01fdff ffffffff 00000000   => logoff command = 0xfffd01e0 (fehlt hier last?)
    // Request 534d4100000402a00000000100220010 606508a0 ffffffffffff0003 7d0042be283a0003 000000000180 e001fdff ffffffff 00000000
    // assemble unicast device logoff packet
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4];
    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer)-20, SpeedwireHeader::sma_inverter_protocol_id);
    request_header.setControl(0xa0);
    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(0xffff);
    request.setDstSerialNumber(0xffffffff);
    request.setDstControl(0x0300);
    request.setSrcSusyID(local_susy_id);
    request.setSrcSerialNumber(local_serial_id);
    request.setSrcControl(0x0300);
    request.setErrorCode(0);
    request.setFragmentID(0);
    request.setPacketID(packet_id);
    request.setCommandID(0xfffd01e0);
    request.setFirstRegisterID(0xffffffff);
    request.setLastRegisterID(0x00000000);
    //request.Trailer(0);  // set trailer
    packet_id = (packet_id + 1) | 0x8000;

    SocketIndex socket_index = socket_map.at(peer.interface_ip_address);
    if (socket_index >= 0) {
        SpeedwireSocket& socket = sockets[socket_index];
        int nsent = socket.sendto(request_buffer, sizeof(request_buffer), peer.peer_ip_address);
        return nsent;
    }
    return -1;
}


int32_t SpeedwireCommand::query(const SpeedwireInfo& peer, const Command command, const uint32_t first_register, const uint32_t last_register, std::vector<SpeedwireRawData>& data) {
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000380 00020058 00348200 ff348200 00000000 =>  query software version
    // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000380 01020058 0a000000 0a000000 01348200 2ae5e65f 00000000 00000000 feffffff feffffff 040a1003 040a1003 00000000 00000000 00000000  code = 0x00823401    3 (BCD).10 (BCD).10 (BIN) Typ R (Enum)
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000480 00020058 001e8200 ff208200 00000000 =>  query device type
    // Response 534d4100000402a000000001009e0010 606527a0 7d0042be283a00a1 7a01842a71b30001 000000000480 01020058 01000000 03000000 011e8210 6f89e95f 534e3a20 33303130 35333831 31360000 00000000 00000000 00000000 00000000 
    //                                                                                                                              011f8208 6f89e95f 411f0001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000 
    //                                                                                                                              01208208 6f89e95f 96240000 80240000 81240001 82240000 feffff00 00000000 00000000 00000000 00000000
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000480 00028053 001e2500 ff1e2500 00000000 =>  query spot dc power
    // Response 534d4100000402a000000001005e0010 606517a0 7d0042be283a00a1 7a01842a71b30001 000000000480 01028053 00000000 01000000 011e2540 61a7e95f 57000000 57000000 57000000 57000000 01000000
    //                                                                                                                              021e2540 61a7e95f 5e000000 5e000000 5e000000 5e000000 01000000 00000000
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000580 00028053 001f4500 ff214500 00000000 =>  query spot dc voltage/current
    // Response 534d4100000402a00000000100960010 606525a0 7d0042be283a00a1 7a01842a71b30001 000000000580 01028053 02000000 05000000 011f4540 61a7e95f 05610000 05610000 05610000 05610000 01000000 
    //                                                                                                                              021f4540 61a7e95f 505b0000 505b0000 505b0000 505b0000 01000000
    //                                                                                                                              01214540 61a7e95f 60010000 60010000 60010000 60010000 01000000
    //                                                                                                                              02214540 61a7e95f 95010000 95010000 95010000 95010000 01000000 00000000
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000680 00020051 00404600 ff424600 00000000 =>  query spot ac power
    // Response 534d4100000402a000000001007a0010 60651ea0 7d0042be283a00a1 7a01842a71b30001 000000000680 01020051 09000000 0b000000 01404640 61a7e95f 38000000 38000000 38000000 38000000 01000000 
    //                                                                                                                              01414640 61a7e95f 37000000 37000000 37000000 37000000 01000000
    //                                                                                                                              01424640 61a7e95f 39000000 39000000 39000000 39000000 01000000 00000000
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000780 00020051 00484600 ff554600 00000000 =>  query spot ac voltage/current
    // Response 534d4100000402a000000001013e0010 60654fa0 7d0042be283a00a1 7a01842a71b30001 000000000780 01020051 0c000000 15000000 01484600 61a7e95f 5a590000 5a590000 5a590000 5a590000 01000000 
    //                                                                                                                              01494600 61a7e95f cf590000 cf590000 cf590000 cf590000 01000000
    //                                                                                                                              014a4600 61a7e95f 7a590000 7a590000 7a590000 7a590000 01000000
    //                                                                                                                              014b4600 61a7e95f f19a0000 f19a0000 f19a0000 f19a0000 01000000
    //                                                                                                                              014c4600 61a7e95f 3c9b0000 3c9b0000 3c9b0000 3c9b0000 01000000
    //                                                                                                                              014d4600 61a7e95f 189b0000 189b0000 189b0000 189b0000 01000000
    //                                                                                                                              014e4600 51a7e95f 1d000000 1d000000 1d000000 1d000000 01000000
    //                                                                                                                              01534640 61a7e95f 24010000 24010000 24010000 24010000 01000000
    //                                                                                                                              01544640 61a7e95f 1e010000 1e010000 1e010000 1e010000 01000000
    //                                                                                                                              01554640 61a7e95f 23010000 23010000 23010000 23010000 01000000 00000000
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000980 00028051 00482100 ff482100 00000000 =>  query device status
    // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000980 01028051 00000000 00000000 01482108 59c5e95f 33010001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000 00000000
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000a80 00028051 00644100 ff644100 00000000 =>  query grid relay status
    // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000a80 01028051 07000000 07000000 01644108 59c5e95f 33000001 37010000 fdffff00 feffff00 00000000 00000000 00000000 00000000 00000000

    // assemble unicast device login packet                                                                                 
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4 + 4];
    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer) - 20, SpeedwireHeader::sma_inverter_protocol_id);
    request_header.setControl(0xa0);
    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(peer.susyID);
    request.setDstSerialNumber(peer.serialNumber);
    request.setDstControl(0x0100);
    request.setSrcSusyID(local_susy_id);
    request.setSrcSerialNumber(local_serial_id);
    request.setSrcControl(0x0100);
    request.setErrorCode(0);
    request.setFragmentID(0);
    request.setPacketID(packet_id);
    request.setCommandID(command);
    request.setFirstRegisterID(first_register);
    request.setLastRegisterID(last_register);
    request.setTrailer(0);
    //printf("query: command %08lx first 0x%08lx last 0x%08lx\n", command, first_register, last_register);

    // send query request packet to peer
    SocketIndex socket_index = socket_map[peer.interface_ip_address];
    if (socket_index < 0) {
        perror("invalid socket_index");
        return -1;
    }
    SpeedwireSocket& socket = sockets[socket_index];
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), peer.peer_ip_address);

    // receive query reply packet from peer
    unsigned char reply_buffer[1500];
    struct sockaddr recvfrom;
    int  nrecv = -1;
    bool valid = false;
    while (valid == false && nrecv != 0) {  // there can be any number of emeter packets in front of the reply packet
        nrecv = recvReply(peer, reply_buffer, sizeof(reply_buffer), 1000, recvfrom);
        valid = checkReply(peer, packet_id, recvfrom, reply_buffer, nrecv);
    }
    packet_id = (packet_id + 1) | 0x8000;
    if (nrecv == 0) {
        perror("receive timeout");
        return -1;
    }
    if (valid == false) {
        perror("invalid reply data or receive error");
        return -1;
    }

    // check error code
    SpeedwireHeader reply_header(reply_buffer, nrecv);
    SpeedwireInverterProtocol reply(reply_header);
    uint16_t result = reply.getErrorCode();
    if (result != 0x0000) {
        if (result == 0x0017) {
            perror("query error code 0x0017 received - lost connection - not authenticated");
        }
        else {
            perror("query error code received");
        }
        return (-1 ^ 0xffff) | result;
    }

    // parse reply packet
    const int speedwire_header_length = reply_header.getPayloadOffset();
    const int inverter_header_length  = 8 + 8 + 6 + 4 + 4 + 4;
    if (reply_header.getLength() > (speedwire_header_length + inverter_header_length + 4 + 8)) {
        uint32_t reply_command  = reply.getCommandID();
        uint32_t reply_first    = reply.getFirstRegisterID();
        uint32_t reply_last     = reply.getLastRegisterID();
        uint32_t payload_length = reply_header.getLength() - inverter_header_length - 4 /*trailer*/;
        uint32_t record_length  = payload_length / (reply_last - reply_first + 1);
        uint32_t record_offset  = 0;
        //printf("=>     command %08lx first 0x%08lx last 0x%08lx rec_len %d\n", reply_command, reply_first, reply_last, record_length);

        // loop across all register ids in the reply packet
        for (uint32_t record = reply_first; record <= reply_last; ++record) {
            if ((record_offset + record_length) > payload_length) {
                perror("payload error");
                return -1;
            }
            SpeedwireRawData record_data(
                (uint32_t)command,
                (uint32_t)(reply.getDataUint32(record_offset) & 0x00ffff00),    // register id
                (uint8_t) (reply.getDataUint32(record_offset) & 0x000000ff),    // connector id (mpp #1, mpp #2, ac #1)
                (uint8_t )(reply.getDataUint32(record_offset) >> 24),           // unknown type
                reply.getDataUint32(record_offset + 4),                         // time
                NULL,                                                           // data
                0);                                                             // data_size
            record_data.data_size = (record_length - 8 < sizeof(record_data.data) ? record_length - 8 : sizeof(record_data.data));
            reply.getDataUint8Array(record_offset + 8, record_data.data, record_data.data_size);
            //printf("=>     %s\n", record_data.toString().c_str());
            data.push_back(record_data);

            record_offset += record_length;
        }
    }

    return (int32_t)data.size();
}


/**
 *  Receive a reply packet from the given peer (or timeout)
 */
int SpeedwireCommand::recvReply(const SpeedwireInfo& peer, void* buff, const size_t buff_size, const unsigned long timeout_in_ms, struct sockaddr &recvfrom) const {

    // determine socket associated with peer
    const SocketIndex socket_index = socket_map.at(peer.interface_ip_address);
    if (socket_index < 0) {
        perror("invalid socket_index");
        return -1;
    }

    // prepare the pollfd structure
    const SpeedwireSocket& socket = sockets[socket_index];
    struct pollfd fds;
    fds.fd      = socket.getSocketFd();
    fds.events  = POLLIN;
    fds.revents = 0;

    // wait for a packet on one of the configured sockets
    if (poll(&fds, 1, timeout_in_ms) < 0) {
        perror("poll failure");
        return -1;
    }

    // determine if the socket received a packet
    if ((fds.revents & POLLIN) != 0) {
        int nbytes = -1;

        // read packet data
        if (socket.isIpv4()) {
            struct sockaddr_in src;
            nbytes = socket.recvfrom(buff, buff_size, src);
            memcpy(&recvfrom, &src, sizeof(src));
        }
        else if (socket.isIpv6()) {
            struct sockaddr_in6 src;
            nbytes = socket.recvfrom(buff, buff_size, src);
            memcpy(&recvfrom, &src, sizeof(src));
        }
        return nbytes;
    }
    return 0;
}


/**
 *  Check reply packet for correctness
 */
bool SpeedwireCommand::checkReply(const SpeedwireInfo& peer, const uint16_t packet_id, const struct sockaddr& recvfrom, const void* const buff, const size_t buff_size) const {
    if (buff_size == 0) return false;                       // timeout
    if (buff == NULL) return false;

    if (buff_size < (20 + 8 + 8 + 6)) {                     // up to and including packetID
        printf("buff_size too small for reply packet:  %u < (20 + 8 + 8 + 6) bytes\n", (unsigned)buff_size);
        return false;
    }

    SpeedwireHeader header(buff, (unsigned)buff_size);
    if (header.checkHeader() == false) {
        printf("checkHeader() failed for speedwire packet\n");
        return false;
    }
    if (header.isInverterProtocolID() == false) {
        printf("protocol ID is not 0x6065\n");
        return false;
    }
    if ((header.getLength() + (size_t)20) > buff_size) {    // packet length - starting to count from the byte following protocolID, # of long words and control byte, i.e. with byte #20
        printf("length field %u and buff_size %u mismatch\n", (unsigned)header.getLength(), (unsigned)buff_size);
        return false;
    }
    if (header.getLength() < (8 + 8 + 6)) {                 // up to and including packetID
        printf("length field %u too small to hold inverter reply (8 + 8 + 6)\n", (unsigned)header.getLength());
        return false;
    }
    if ((header.getLongWords() != (header.getLength() / sizeof(uint32_t)))) {
        printf("length field %u and long words %u mismatch\n", (unsigned)header.getLength(), (unsigned)header.getLongWords());
        return false;
    }

    SpeedwireInverterProtocol inverter(header);
    if (inverter.getDstSusyID() != 0xffff && inverter.getDstSusyID() != local_susy_id) {
        printf("destination susy id %u is not local susy id %u\n", (unsigned)inverter.getDstSusyID(), (unsigned)local_susy_id);
        return false;
    }
    if (inverter.getDstSerialNumber() != 0xffffffff && inverter.getDstSerialNumber() != local_serial_id) {
        printf("destination serial number %u is not local serial number %u\n", (unsigned)inverter.getDstSerialNumber(), (unsigned)local_serial_id);
        return false;
    }
    if (inverter.getSrcSusyID() != peer.susyID) {
        printf("source susy id %u is not peer susy id %u\n", (unsigned)inverter.getSrcSusyID(), (unsigned)peer.susyID);
        return false;
    }
    if (inverter.getSrcSerialNumber() != peer.serialNumber) {
        printf("source serial number %u is not peer serial number %u\n", (unsigned)inverter.getSrcSerialNumber(), (unsigned)peer.serialNumber);
        return false;
    }
    if (inverter.getPacketID() != packet_id) {
        printf("reply packet id %u is not equal request packet id %u\n", (unsigned)inverter.getPacketID(), (unsigned)packet_id);
        return false;
    }

    if (recvfrom.sa_family == AF_INET) {
        const struct sockaddr_in *const addr = (const struct sockaddr_in* const)&recvfrom;
        if (addr->sin_port != htons(9522)) {
            printf("ipv4 port %u is not 9522\n", (unsigned)ntohs(addr->sin_port));
            return false;
        }
        if (addr->sin_addr.s_addr != localhost.toInAddress(peer.peer_ip_address).s_addr) {
            printf("ipv4 address %s is not peer ip address %s\n", LocalHost::toString(addr->sin_addr).c_str(), peer.peer_ip_address.c_str());
            return false;
        }
    }
    else if (recvfrom.sa_family == AF_INET6) {
        const struct sockaddr_in6* const addr = (const struct sockaddr_in6* const)&recvfrom;
        if (addr->sin6_port != htons(9522)) {
            printf("ipv6 port %u is not 9522\n", (unsigned)ntohs(addr->sin6_port));
            return false;
        }
        struct in6_addr temp = localhost.toIn6Address(peer.peer_ip_address);
        if (memcmp(&addr->sin6_addr, &temp, sizeof(temp)) != 0) {
            printf("ipv4 address %s is not peer ip address %s\n", LocalHost::toString(addr->sin6_addr).c_str(), peer.peer_ip_address.c_str());
            return false;
        }
    }

    return true;
}
