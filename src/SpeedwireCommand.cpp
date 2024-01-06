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
#include <LocalHost.hpp>
#include <AddressConversion.hpp>
#include <Logger.hpp>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireDevice.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>
#include <SpeedwireCommand.hpp>
using namespace libspeedwire;

static Logger logger("SpeedwireCommand");

const uint16_t SpeedwireCommand::local_susy_id   = SpeedwireDevice::getLocalDevice().susyID;
const uint32_t SpeedwireCommand::local_serial_id = SpeedwireDevice::getLocalDevice().serialNumber;

uint16_t SpeedwireCommand::packet_id = 0x8001;


SpeedwireCommand::SpeedwireCommand(const LocalHost &_localhost, const std::vector<SpeedwireDevice> &_devices) :
    localhost(_localhost),
    devices(_devices) {
    // loop across all speedwire devices
    for (auto& device : devices) {
        // check if there is already a map entry for the interface ip address
        if (socket_map.find(device.interface_ip_address) == socket_map.end() && device.interface_ip_address.length() > 0 && device.interface_ip_address != "0.0.0.0") {
            // create and open a socket for the interface
            SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getRecvSocket(SpeedwireSocketFactory::SocketType::UNICAST, device.interface_ip_address);
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
    //packet_id = 0x8001; // | (uint16_t)LocalHost::getUnixEpochTimeInMs();
}

SpeedwireCommand::~SpeedwireCommand(void) {
    sockets.clear();
    socket_map.clear();
}


/**
 *  synchronous login method - send inverter login command to the given destination peer, wait for the response and check for error codes
 */
bool SpeedwireCommand::login(const SpeedwireDevice& dst_peer, const bool user, const char* password, const int timeout_in_ms) {
    return login(dst_peer, SpeedwireDevice::getLocalDevice(), user, password, timeout_in_ms);
}


/**
 *  synchronous login method - send inverter login command to the given peer, wait for the response and check for error codes
 */
bool SpeedwireCommand::login(const SpeedwireDevice& dst_peer, const SpeedwireDevice& src_peer, const bool user, const char* password, const int timeout_in_ms) {

    // determine receive socket
    SocketIndex socket_index = socket_map[dst_peer.interface_ip_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return false;
    }
    SpeedwireSocket& socket = sockets[socket_index];

    // send login request to peer
    SpeedwireCommandTokenIndex token_index = sendLoginRequest(dst_peer, src_peer, user, password);
    if (token_index < 0) {
        return false;
    }

    // wait for the response
    unsigned char response_buffer[2048];
    int32_t nbytes = receiveResponse(token_index, socket, response_buffer, sizeof(response_buffer), timeout_in_ms);
    if (nbytes <= 0) {
        return false;
    }

    // check error code
    const SpeedwireHeader speedwire_packet(response_buffer, sizeof(response_buffer));
    if (speedwire_packet.isValidData2Packet()) {
        const SpeedwireData2Packet data2_packet(speedwire_packet);

        if (data2_packet.isInverterProtocolID()) {
            const SpeedwireInverterProtocol inverter_packet(data2_packet);

            uint16_t error_code = inverter_packet.getErrorCode();
            if (error_code != 0x0000) {
                if (error_code == 0x0017) {
                    logger.print(LogLevel::LOG_ERROR, "lost connection - not authenticated (error code 0x0017)");
                    token_repository.needs_login = true;
                }
                else if (token_repository.at(token_index).command == 0xfffd040c) { // login command
                    if (error_code == 0x0100) {
                        logger.print(LogLevel::LOG_ERROR, "invalid password - not authenticated");
                    }
                    else {
                        logger.print(LogLevel::LOG_ERROR, "login failure - not authenticated");
                    }
                }
                else {
                    logger.print(LogLevel::LOG_ERROR, "query error code received");
                }
                token_repository.remove(token_index);
                return false;
            }
        }
    }
    token_repository.remove(token_index);
    return true;
}


/**
 *  synchronous logoff method - send inverter logoff command to the given peer and return; there is no reply from the inverter for logoff commands
 */
bool SpeedwireCommand::logoff(const SpeedwireDevice& dst_peer) {
    logoff(dst_peer, SpeedwireDevice::getLocalDevice());
    return true;
}


/**
 *  synchronous logoff method - send inverter logoff command to the given peer and return; there is no reply from the inverter for logoff commands
 */
bool SpeedwireCommand::logoff(const SpeedwireDevice& dst_peer, const SpeedwireDevice& src_peer) {
    sendLogoffRequest(dst_peer, src_peer);
    return true;
}


/**
 *  synchronous query method - send inverter query command to the given peer, wait for the response and check for error codes
 *  this method cannot handle fragmented response packets
 */
int32_t SpeedwireCommand::query(const SpeedwireDevice& peer, const Command command, const uint32_t first_register, const uint32_t last_register, void* udp_buffer, const size_t udp_buffer_size, const int timeout_in_ms) {

    // determine receive socket
    SocketIndex socket_index = socket_map[peer.interface_ip_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return -1;
    }
    SpeedwireSocket& socket = sockets[socket_index];

    // send query request to peer
    SpeedwireCommandTokenIndex token_index = sendQueryRequest(peer, command, first_register, last_register);
    if (token_index < 0) {
        return -1;
    }

    // wait for the response
    int32_t nbytes = receiveResponse(token_index, socket, udp_buffer, udp_buffer_size, timeout_in_ms);
    if (nbytes <= 0) {
        return -1;
    }

    // check error code
    const SpeedwireHeader speedwire_packet(udp_buffer, (unsigned long)udp_buffer_size);
    const SpeedwireInverterProtocol inverter_packet(speedwire_packet);
    uint16_t error_code = inverter_packet.getErrorCode();
    if (error_code != 0x0000) {
        if (error_code == 0x0017) {
            logger.print(LogLevel::LOG_ERROR, "lost connection - not authenticated (error code 0x0017)");
            token_repository.needs_login = true;
        }
        else {
            logger.print(LogLevel::LOG_ERROR, "query error code received");
        }
        token_repository.remove(token_index);
        return -1;
    }
    token_repository.remove(token_index);
    return nbytes;
}


/**
 *  send inverter login command to the given peer
 */
SpeedwireCommandTokenIndex SpeedwireCommand::sendLoginRequest(const SpeedwireDevice& dst_peer, const SpeedwireDevice& src_peer, const bool user, const char* password) {
    // Request  534d4100000402a000000001003a0010 60650ea0 7a01842a71b30001 7d0042be283a0001 000000000280 0c04fdff 07000000 84030000 00d8e85f 00000000 c1c1c1c18888888888888888 00000000   => login command = 0xfffd040c, first = 0x00000007 (user 7, installer a), last = 0x00000384 (hier timeout), time = 0x5fdf9ae8, 0x00000000, pw 12 bytes
    // Response 534d4100000402a000000001002e0010 60650be0 7d0042be283a0001 7a01842a71b30001 000000000280 0d04fdff 07000000 84030000 00d8e85f 00000000 00000000 => login OK
    // Response 534d4100000402a000000001002e0010 60650be0 7d0042be283a0001 7a01842a71b30001 000100000280 0d04fdff 07000000 84030000 fddbe85f 00000000 00000000 => login INVALID PASSWORD
    // command  0xfffd040c => 0x400 set?  0x00c bytecount=12?
    // assemble unicast device login packet
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4 + 4 + 4 + 12 + 4];
    memset(request_buffer, 0, sizeof(request_buffer));

    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer) - 20, SpeedwireData2Packet::sma_inverter_protocol_id);   // 4 + 8 + 4 => es fehlen 4 byte an der Länge, prüfen of tagLength erst nach dem control word zählt
    //LocalHost::hexdump(request_buffer, sizeof(request_buffer));

    SpeedwireData2Packet data2_packet(request_header);
    data2_packet.setControl(0xa0);

    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(dst_peer.susyID);
    request.setDstSerialNumber(dst_peer.serialNumber);
    request.setDstControl(0x0100);
    request.setSrcSusyID(src_peer.susyID);
    request.setSrcSerialNumber(src_peer.serialNumber);
    request.setSrcControl(0x0100);
    request.setErrorCode(0);
    request.setFragmentCounter(0);
    request.setPacketID(packet_id);
    request.setCommandID(0xfffd040c);
    request.setFirstRegisterID((user ? 0x00000007 : 0x0000000a));    // user: 0x7  installer: 0xa
    request.setLastRegisterID(0x00000384);     // timeout
    request.setDataUint32(0, (uint32_t)time(NULL));
    request.setDataUint32(4, 0x00000000);
    uint8_t encoded_password[12];
    memset(encoded_password, (user ? 0x88 : 0xBB), sizeof(encoded_password));
    for (int i = 0; password[i] != '\0' && i < sizeof(encoded_password); ++i) {
        encoded_password[i] = password[i] + (user ? 0x88 : 0xBB);
    }
    request.setDataUint8Array(8, encoded_password, sizeof(encoded_password));

    // send login request packet to peer
    SocketIndex socket_index = socket_map[dst_peer.interface_ip_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return -1;
    }
    SpeedwireSocket& socket = sockets[socket_index];
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), dst_peer.peer_ip_address);
    if (nsent <= 0) {
        logger.print(LogLevel::LOG_ERROR, "cannot send data to socket");
        return -1;
    }

    // add a query token; this is used to match reply packets to this request packet
    SpeedwireCommandTokenIndex index = token_repository.add(dst_peer.susyID, dst_peer.serialNumber, packet_id, dst_peer.peer_ip_address, 0xfffd040c);

    // increment packet id
    packet_id = (packet_id + 1) | 0x8000;

    return index;
}


/**
 *  send inverter logoff command to the given peer
 */
void SpeedwireCommand::sendLogoffRequest(const SpeedwireDevice& dst_peer, const SpeedwireDevice& src_peer) {
    // Request 534d4100000402a00000000100220010 606508a0 ffffffffffff0003 7d0052be283a0003 000000000280 0e01fdff ffffffff 00000000   => logoff command = 0xfffd01e0 (fehlt hier last?)
    // Request 534d4100000402a00000000100220010 606508a0 ffffffffffff0003 7d0042be283a0003 000000000180 e001fdff ffffffff 00000000
    // assemble unicast device logoff packet
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4];
    memset(request_buffer, 0, sizeof(request_buffer));

    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer)-20, SpeedwireData2Packet::sma_inverter_protocol_id);

    SpeedwireData2Packet data2_packet(request_header);
    data2_packet.setControl(0xa0);

    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(0xffff);
    request.setDstSerialNumber(0xffffffff);
    request.setDstControl(0x0300);
    request.setSrcSusyID(src_peer.susyID);
    request.setSrcSerialNumber(src_peer.serialNumber);
    request.setSrcControl(0x0300);
    request.setErrorCode(0);
    request.setFragmentCounter(0);
    request.setPacketID(packet_id);
    request.setCommandID(0xfffd01e0);
    request.setFirstRegisterID(0xffffffff);
    request.setLastRegisterID(0x00000000);

    SocketIndex socket_index = socket_map.at(dst_peer.interface_ip_address);
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return;
    }
    SpeedwireSocket& socket = sockets[socket_index];
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), dst_peer.peer_ip_address);
    if (nsent <= 0) {
        logger.print(LogLevel::LOG_ERROR, "cannot send data to socket");
        return;
    }

    // increment packet id
    packet_id = (packet_id + 1) | 0x8000;
}


/**
 *  assemble inverter query command and send it to the given peer
 */
SpeedwireCommandTokenIndex SpeedwireCommand::sendQueryRequest(const SpeedwireDevice& peer, const Command command, const uint32_t first_register, const uint32_t last_register) {
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000380 00020058 00348200 ff348200 00000000 =>  query software version
    // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000380 01020058 0a000000 0a000000 01348200 2ae5e65f 00000000 00000000 feffffff feffffff 040a1003 040a1003 00000000 00000000 00000000  code = 0x00823401    3 (BCD).10 (BCD).10 (BIN) Typ R (Enum)
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000480 00020058 001e8200 ff208200 00000000 =>  query device type
    // Response 534d4100000402a000000001009e0010 606527a0 7d0042be283a00a1 7a01842a71b30001 000000000480 01020058 01000000 03000000 011e8210 6f89e95f 534e3a20 33303130 35333831 31360000 00000000 00000000 00000000 00000000 
    //                                                                                                                              011f8208 6f89e95f 411f0001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000  => 1f41 solar inverter
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
    memset(request_buffer, 0, sizeof(request_buffer));

    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer) - 20, SpeedwireData2Packet::sma_inverter_protocol_id);

    SpeedwireData2Packet data2_packet(request_header);
    data2_packet.setControl(0xa0);
    //LocalHost::hexdump(request_buffer, sizeof(request_buffer));

    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(peer.susyID);
    request.setDstSerialNumber(peer.serialNumber);
    request.setDstControl(0x0100);
    request.setSrcSusyID(local_susy_id);
    request.setSrcSerialNumber(local_serial_id);
    request.setSrcControl(0x0100);
    request.setErrorCode(0);
    request.setFragmentCounter(0);
    request.setPacketID(packet_id);
    request.setCommandID(command);
    request.setFirstRegisterID(first_register);
    request.setLastRegisterID(last_register);
    //printf("query: command %08lx first 0x%08lx last 0x%08lx\n", command, first_register, last_register);

    // send query request packet to peer
    SocketIndex socket_index = socket_map[peer.interface_ip_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return -1;
    }
    SpeedwireSocket& socket = sockets[socket_index];
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), peer.peer_ip_address);
    if (nsent <= 0) {
        logger.print(LogLevel::LOG_ERROR, "cannot send data to socket");
        return -1;
    }

    // add a query token; this is used to match reply packets to this request packet
    SpeedwireCommandTokenIndex index = token_repository.add(peer.susyID, peer.serialNumber, packet_id, peer.peer_ip_address, command);

    // increment packet id
    packet_id = (packet_id + 1) | 0x8000;

    return index;
}


/**
 *  query device type
 */
SpeedwireDevice SpeedwireCommand::queryDeviceType(const SpeedwireDevice& peer, const int timeout_in_ms) {
    // create a copy of the peer
    SpeedwireDevice info = peer;

    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000380 00020058 00348200 ff348200 00000000 =>  query software version
    // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000380 01020058 0a000000 0a000000 01348200 2ae5e65f 00000000 00000000 feffffff feffffff 040a1003 040a1003 00000000 00000000 00000000  code = 0x00823401    3 (BCD).10 (BCD).10 (BIN) Typ R (Enum)
    // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000480 00020058 001e8200 ff208200 00000000 =>  query device type
    // Response 534d4100 000402a000000001 009e0010 606527a0 7d0042be283a00a1 7a01842a71b30001 000000000480 01020058 01000000 03000000 011e8210 6f89e95f 534e3a20 33303130 35333831 31360000 00000000 00000000 00000000 00000000 
    //                                                                                                                              011f8208 6f89e95f 411f0001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000  => 1f41 solar inverter
    //                                                                                                                              01208208 6f89e95f 96240000 80240000 81240001 82240000 feffff00 00000000 00000000 00000000 00000000
    // send unicast query device type request
    SpeedwireCommandTokenIndex token_index = sendQueryRequest(peer, Command::COMMAND_DEVICE_QUERY, 0x00821E00, 0x008220FF);
    //SpeedwireCommandTokenIndex token_index = sendQueryRequest(peer, Command::COMMAND_DEVICE_QUERY, 0x00823400, 0x008234FF);  // query software version

    // determine socket
    SocketIndex socket_index = socket_map[peer.interface_ip_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return info;
    }
    SpeedwireSocket& socket = sockets[socket_index];

    // wait for response
    unsigned char udp_packet[2048];
    int32_t nbytes = receiveResponse(token_index, socket, udp_packet, sizeof(udp_packet), timeout_in_ms);

    if (nbytes == 0) {
        if (peer.deviceClass != toString(SpeedwireDeviceClass::EMETER)) {
            printf("timeout in queryDeviceType() for %s via %s\n", peer.peer_ip_address.c_str(), socket.getLocalInterfaceAddress().c_str());
        }
    }
    else if (nbytes > 0) {
        //LocalHost::hexdump(udp_packet, nbytes);

        // parse reply packet
        SpeedwireHeader speedwire_packet(udp_packet, nbytes);
        if (speedwire_packet.isValidData2Packet()) {
            if (!speedwire_packet.isValidData2Packet(true)) {
                printf("is valid speedwire packet, but minor deviations from standard detected\n");
            }

            SpeedwireData2Packet data2_packet(speedwire_packet);
            if (data2_packet.isInverterProtocolID()) {

                SpeedwireInverterProtocol inverter_packet(data2_packet);
                //LocalHost::hexdump(udp_packet, nbytes);
                //printf("%s\n", inverter_packet.toString().c_str());

                std::vector<SpeedwireRawData> raw_data_vector = inverter_packet.getRawDataElements();

                // augment the device information with data obtained the peer
                for (auto& raw_data : raw_data_vector) {
                    if (raw_data.id == SpeedwireData::InverterDeviceClass.id && raw_data.type == SpeedwireDataType::Status32) {
                        SpeedwireRawDataStatus32 status_data(raw_data);
                        size_t index = status_data.getSelectionIndex();
                        if (index != (size_t)-1) {
                            SpeedwireDeviceClass device_class = (SpeedwireDeviceClass)status_data.getValue(index);;
                            info.deviceClass = libspeedwire::toString(device_class);
                        }
                    }
                    else if (raw_data.id == SpeedwireData::InverterDeviceType.id && raw_data.type == SpeedwireDataType::Status32) {
                        SpeedwireRawDataStatus32 status_data(raw_data);
                        size_t index = status_data.getSelectionIndex();
                        if (index != (size_t)-1) {
                            SpeedwireDeviceModel device_model = (SpeedwireDeviceModel)status_data.getValue(index);;
                            SpeedwireDeviceType device = SpeedwireDeviceType::fromDeviceModel(device_model);
                            info.deviceModel = device.name;
                        }
                    }
                }
            }
        }
    }
    //printf("%s\n", info.toString().c_str());
    return info;
}


/**
 *  synchronously receive inverter reply; for asynchronous receiption please use class SpeedwireReceiveDispatcher
 */
int32_t SpeedwireCommand::receiveResponse(const SpeedwireCommandTokenIndex token_index, SpeedwireSocket& socket, void* udp_buffer, const size_t udp_buffer_size, const int timeout_in_ms) {

    // prepare the pollfd structure
    struct pollfd pollfds;
    pollfds.fd      = socket.getSocketFd();
    pollfds.events  = POLLIN;
    pollfds.revents = 0;

    // enter packet receive wait loop - any udp packets received before the inverter packet or before the timeout kicks in are skipped(!)
    int  nbytes = -1;
    bool valid  = false;
    while (valid == false && nbytes != 0) {

        // wait for a packet on the configured socket
        int pollresult = poll(&pollfds, 1, timeout_in_ms);
        if (pollresult == 0) {
            //perror("poll timeout in SpeedwireCommand");
            return 0;
        }
        if (pollresult < 0) {
            logger.print(LogLevel::LOG_ERROR, "poll failure");
            return -1;
        }

        // determine if the socket received a packet
        if ((pollfds.revents & POLLIN) != 0) {

            // read packet data
            struct sockaddr src;
            if (socket.isIpv4()) {
                nbytes = socket.recvfrom(udp_buffer, udp_buffer_size, AddressConversion::toSockAddrIn(src));
            }
            else if (socket.isIpv6()) {
                nbytes = socket.recvfrom(udp_buffer, udp_buffer_size, AddressConversion::toSockAddrIn6(src));
            }

            // check if the reply is a valid sma speedwire data2 packet
            SpeedwireHeader speedwire_packet(udp_buffer, nbytes);
            if (speedwire_packet.isValidData2Packet()) {
                if (!speedwire_packet.isValidData2Packet(true)) {
                    printf("is valid speedwire packet, but minor deviations from standard detected\n");
                }
                SpeedwireData2Packet data2_packet(speedwire_packet);

                // check if the reply is an inverter packet
                if (data2_packet.isInverterProtocolID()) {

                    // check reply packet for validity
                    const SpeedwireCommandToken& token = token_repository.at(token_index);
                    if (checkReply(speedwire_packet, src, token) == true) {
                        valid = true;
                        token_repository.remove(token_index);
                    }
                }
            }
        }
    }
    return nbytes;
}


/**
 *  find SpeedwireCommandToken for the reply packet; return index in token_repository or -1
 */
int SpeedwireCommand::findCommandToken(const SpeedwireHeader& speedwire_reply_packet) const {
    if (speedwire_reply_packet.isValidData2Packet()) {
        const SpeedwireData2Packet data2_packet(speedwire_reply_packet);

        if (data2_packet.isInverterProtocolID()) {
            const SpeedwireInverterProtocol inverter_packet(data2_packet);

            uint16_t susyid = inverter_packet.getSrcSusyID();
            uint32_t serial = inverter_packet.getSrcSerialNumber();
            uint16_t packetid = inverter_packet.getPacketID();

            return token_repository.find(susyid, serial, packetid);
        }
    }
    return -1;
}


/**
 *  get a reference to the repository of active SpeedwireCommandTokens
 */
SpeedwireCommandTokenRepository& SpeedwireCommand::getTokenRepository(void) {
    return token_repository;
}


/**
 *  check reply packet for correctness
 */
bool SpeedwireCommand::checkReply(const SpeedwireHeader& speedwire_reply_packet, const struct sockaddr& recvfrom) const {
    int token_index = findCommandToken(speedwire_reply_packet);
    if (token_index < 0) {
        //logger.print(LogLevel::LOG_ERROR, "cannot find query token => DROPPED\n");
        return false;
    }
    const SpeedwireCommandToken& token = token_repository.at(token_index);
    return checkReply(speedwire_reply_packet, recvfrom, token);
}

bool SpeedwireCommand::checkReply(const SpeedwireHeader& speedwire_reply_packet, const struct sockaddr& recvfrom, const SpeedwireCommandToken& token) {
    size_t   buff_size = speedwire_reply_packet.getPacketSize();
    uint8_t* buff      = speedwire_reply_packet.getPacketPointer();

    if (buff_size == 0) return false;                       // timeout
    if (buff == NULL) return false;

    if (buff_size < (20 + 8 + 8 + 6)) {                     // up to and including packetID
        printf("buff_size too small for reply packet:  %u < (20 + 8 + 8 + 6) bytes\n", (unsigned)buff_size);
        return false;
    }
    if (!speedwire_reply_packet.isValidData2Packet()) {
        printf("isValidData2Packet() failed for speedwire packet\n");
    }

    const SpeedwireData2Packet data2_reply_packet(speedwire_reply_packet);
    if (data2_reply_packet.isInverterProtocolID() == false) {
        printf("protocol ID is not 0x6065\n");
        return false;
    }
    if ((data2_reply_packet.getTagLength() + (size_t)20) > buff_size) {    // packet length - starting to count from the byte following protocolID, # of long words and control byte, i.e. with byte #20
        printf("length field %u and buff_size %u mismatch\n", (unsigned)data2_reply_packet.getTagLength(), (unsigned)buff_size);
        return false;
    }
    if (data2_reply_packet.getTagLength() < (8 + 8 + 6)) {                 // up to and including packetID
        printf("length field %u too small to hold inverter reply (8 + 8 + 6)\n", (unsigned)data2_reply_packet.getTagLength());
        return false;
    }
    if ((data2_reply_packet.getLongWords() != (data2_reply_packet.getTagLength() / sizeof(uint32_t)))) {
        printf("length field %u and long words %u mismatch\n", (unsigned)data2_reply_packet.getTagLength(), (unsigned)data2_reply_packet.getLongWords());
        return false;
    }

    const SpeedwireInverterProtocol inverter(data2_reply_packet);
    if (inverter.getDstSusyID() != 0xffff && inverter.getDstSusyID() != local_susy_id) {
        printf("destination susy id %u is not local susy id %u\n", (unsigned)inverter.getDstSusyID(), (unsigned)local_susy_id);
        return false;
    }
    if (inverter.getDstSerialNumber() != 0xffffffff && inverter.getDstSerialNumber() != local_serial_id) {
        printf("destination serial number %u is not local serial number %u\n", (unsigned)inverter.getDstSerialNumber(), (unsigned)local_serial_id);
        return false;
    }
    if (inverter.getSrcSusyID() != token.susyid) {
        printf("source susy id %u is not peer susy id %u\n", (unsigned)inverter.getSrcSusyID(), (unsigned)token.susyid);
        return false;
    }
    if (inverter.getSrcSerialNumber() != token.serialnumber) {
        printf("source serial number %u is not peer serial number %u\n", (unsigned)inverter.getSrcSerialNumber(), (unsigned)token.serialnumber);
        return false;
    }
    if ((inverter.getPacketID() | 0x8000) != token.packetid) {
        printf("reply packet id %u is not equal request packet id %u\n", (unsigned)inverter.getPacketID(), (unsigned)token.packetid);
        return false;
    }

    if (recvfrom.sa_family == AF_INET) {
        const struct sockaddr_in& addr = AddressConversion::toSockAddrIn(recvfrom);
        if (addr.sin_port != htons(9522)) {
            printf("ipv4 port %u is not 9522\n", (unsigned)ntohs(addr.sin_port));
            return false;
        }
        if (addr.sin_addr.s_addr != AddressConversion::toInAddress(token.peer_ip_address).s_addr) {
            printf("ipv4 address %s is not peer ip address %s\n", AddressConversion::toString(addr.sin_addr).c_str(), token.peer_ip_address.c_str());
            return false;
        }
    }
    else if (recvfrom.sa_family == AF_INET6) {
        const struct sockaddr_in6& addr = AddressConversion::toSockAddrIn6(recvfrom);
        if (addr.sin6_port != htons(9522)) {
            printf("ipv6 port %u is not 9522\n", (unsigned)ntohs(addr.sin6_port));
            return false;
        }
        struct in6_addr temp = AddressConversion::toIn6Address(token.peer_ip_address);
        if (memcmp(&addr.sin6_addr, &temp, sizeof(temp)) != 0) {
            printf("ipv4 address %s is not peer ip address %s\n", AddressConversion::toString(addr.sin6_addr).c_str(), token.peer_ip_address.c_str());
            return false;
        }
    }

    return true;
}


//=====================================================================================

SpeedwireCommandTokenIndex SpeedwireCommandTokenRepository::add(const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid, const std::string& peer_ip_address, const uint32_t command) {
    uint32_t create_time = (uint32_t)LocalHost::getUnixEpochTimeInMs();
    SpeedwireCommandToken new_token = { susyid, serialnumber, packetid, peer_ip_address, command, create_time };
    token.push_back(new_token);
    return (SpeedwireCommandTokenIndex)token.size()-1;
}

void SpeedwireCommandTokenRepository::remove(const SpeedwireCommandTokenIndex index) {
    int i = 0;
    for (std::vector<SpeedwireCommandToken>::iterator it = token.begin(); it != token.end(); ++it, ++i) {
        if (i == index) {
            it = token.erase(it);
            break;
        }
    }
}

int SpeedwireCommandTokenRepository::find(const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid) const {
    for (int i = 0; i < token.size(); ++i) {
        const SpeedwireCommandToken& t = token[i];
        if (t.susyid == susyid && t.serialnumber == serialnumber && t.packetid == (packetid | 0x8000)) {
            return i;
        }
    }
    return -1;
}

const SpeedwireCommandToken& SpeedwireCommandTokenRepository::at(const SpeedwireCommandTokenIndex index) const {
    return token[index];
}

void SpeedwireCommandTokenRepository::clear(void) {
    token.clear();
}

int SpeedwireCommandTokenRepository::expire(const int timeout_in_ms) {
    uint32_t now = (uint32_t)LocalHost::getUnixEpochTimeInMs();
    int count = 0;
    for (std::vector<SpeedwireCommandToken>::iterator it = token.begin(); it != token.end(); /*nop*/) {
        if ((now - it->create_time) > (uint32_t)timeout_in_ms) {
            it = token.erase(it);
            ++count;
        }
        else {
            ++it;
        }
    }
    return count;
}

int SpeedwireCommandTokenRepository::size(void) const {
    return (int)token.size();
}
