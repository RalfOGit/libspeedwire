#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireInverterProtocol.hpp>
using namespace libspeedwire;


//SpeedwireInverterProtocol::SpeedwireInverterProtocol(const void* const udp_packet, const unsigned long udp_packet_len) {
//    udp = (uint8_t*)udp_packet;
//    size = udp_packet_len;
//    packet_id = 0x8001;
//}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireHeader instance that encapsulate the SMA header and the pointers to the entire udp packet.
 */
SpeedwireInverterProtocol::SpeedwireInverterProtocol(const SpeedwireHeader& header) :
    SpeedwireInverterProtocol(SpeedwireData2Packet(header)) {
}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireData2Packet instance that encapsulate the data2 tag header.
 */
SpeedwireInverterProtocol::SpeedwireInverterProtocol(const SpeedwireData2Packet& data2_packet) {
    //uint16_t tag_length  = data2_packet.getTagLength();     // 2 bytes
    //uint16_t tag_id      = data2_packet.getTagId();         // 2 bytes
    //uint16_t protocol_id = data2_packet.getProtocolID();    // 2 bytes
    //uint8_t  long_words  = data2_packet.getLongWords();     // 1 byte
    //uint8_t  control     = data2_packet.getControl();       // 1 byte
    unsigned long payload_offset = data2_packet.getPayloadOffset(); // returns 2 + 2 + 2 + 1 + 1 = 8
    udp  = data2_packet.getPacketPointer() + payload_offset;        // udp points to the byte after the control byte
    size = data2_packet.getTotalLength() - payload_offset;
}


/** Destructor. */
SpeedwireInverterProtocol::~SpeedwireInverterProtocol(void) {
    udp = NULL;
    size = 0;
}


/** Get destination susy id. */
uint16_t SpeedwireInverterProtocol::getDstSusyID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_dst_susy_id_offset);
}

/** Get destination serial number. */
uint32_t SpeedwireInverterProtocol::getDstSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_dst_serial_number_offset);
}

/** Get destination control word. */
uint16_t SpeedwireInverterProtocol::getDstControl(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_dst_control_offset);
}

/** Get source susy id. */
uint16_t SpeedwireInverterProtocol::getSrcSusyID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_src_susy_id_offset);
}

/** Get source serial number. */
uint32_t SpeedwireInverterProtocol::getSrcSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_src_serial_number_offset);
}

/** Get source control word. */
uint16_t SpeedwireInverterProtocol::getSrcControl(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_src_control_offset);
}

/** Get error code. */
uint16_t SpeedwireInverterProtocol::getErrorCode(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_error_code_offset);
}

/** Get fragment counter. */
uint16_t SpeedwireInverterProtocol::getFragmentCounter(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_fragment_counter_offset);
}

/** Get packet id. */
uint16_t SpeedwireInverterProtocol::getPacketID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_packet_id_offset);
}

/** Get command id. */
uint32_t SpeedwireInverterProtocol::getCommandID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_command_id_offset);
}

/** Get first register id. */
uint32_t SpeedwireInverterProtocol::getFirstRegisterID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_first_register_id_offset);
}

/** Get last register id. */
uint32_t SpeedwireInverterProtocol::getLastRegisterID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_last_register_id_offset);
}


/** Get 32-bit of data from the given offset in the data area. */
uint32_t SpeedwireInverterProtocol::getDataUint32(unsigned long byte_offset) const {   // offset 0 is the first byte after last register index
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_data_offset + byte_offset);
}

/** Get 64-bit of data from the given offset in the data area. */
uint64_t SpeedwireInverterProtocol::getDataUint64(unsigned long byte_offset) const {
    return SpeedwireByteEncoding::getUint64LittleEndian(udp + sma_data_offset + byte_offset);
}

/** Get an array of 8-bit data from the given offset in the data area. */
void SpeedwireInverterProtocol::getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const {
    if (buff != NULL) {
        memcpy(buff, udp + sma_data_offset + byte_offset, buff_size);
    }
}

/**
 * Get length of the given raw data element. 
 * It is assumed that all raw data elements in a given inverter packet have the same size. Then the size of each element can 
 * be calculated from the number of elements and the size of the inverter packet payload.
 * @returns the size of each raw data element or 0 if the assumption does not hold
 */
uint32_t SpeedwireInverterProtocol::getRawDataLength(void) const {
    uint32_t first_register_id = getFirstRegisterID();  // arbitrary code of the first data element
    uint32_t last_register_id  = getLastRegisterID();   // arbitrary code of the last data element
    if (last_register_id >= first_register_id) {
        uint32_t num_register_ids = last_register_id - first_register_id + 1;   // number of data elements
        uint32_t register_payload_size = size - sma_data_offset;                // payload size
        // check if the data elements exactly fit into the payload size
        if (register_payload_size > 0 && num_register_ids != 0 && (register_payload_size % num_register_ids) == 0) {
            uint32_t length = register_payload_size / num_register_ids;         // length of each data element
            // check if at least a code word, a timestamp and one data word would fit into each data element
            if (length >= 12) {
                return length;
            }
        }
    }
    // if any of the assumptions above won't fit, return ß
    return 0;
}

/** Get pointer to first raw data element in udp packet. */
const void* SpeedwireInverterProtocol::getFirstRawDataElement(void) const {
    return udp + sma_data_offset;
}

/** Get pointer to next raw data element starting from the given element. */
const void* SpeedwireInverterProtocol::getNextRawDataElement(const void* const current_element, uint32_t element_length) const {
    uint8_t* const next_element = ((uint8_t* const)current_element) + element_length;
    // check if the next element including the 4-byte head is inside the udp packet
    if ((std::uintptr_t)(next_element + 4 - udp) > size) {
        return NULL;
    }
    // check if the next element is a trailer word, i.e. all '0's
    if (SpeedwireByteEncoding::getUint32LittleEndian(next_element) == 0x00000000) {
        return NULL;
    }
    // check if the entire next element is inside the udp packet
    if ((std::uintptr_t)(next_element + element_length - udp) > size) {
        return NULL;
    }
    return next_element;
}

/** Get raw data from the given raw data element. */
SpeedwireRawData SpeedwireInverterProtocol::getRawData(const void* const current_element, uint32_t element_length) const {
    uint32_t first_word  = 0xffffffff;
    uint32_t second_word = 0xffffffff;
    if (current_element != NULL) {
        first_word  = SpeedwireByteEncoding::getUint32LittleEndian(current_element);
        second_word = SpeedwireByteEncoding::getUint32LittleEndian((uint8_t*)current_element + 4);
    }
    SpeedwireRawData raw_data = {
        first_word,                             // command
        (uint32_t)(first_word & 0x00ffff00),    // register id;
        (uint8_t) (first_word & 0x000000ff),    // connector id (mpp #1, mpp #2, ac #1);
        SpeedwireDataType(first_word >> 24),    // type;
        second_word,                            // time;
        NULL,
        (element_length - 8 < sizeof(SpeedwireRawData::data) ? element_length - 8 : sizeof(SpeedwireRawData::data))
    };
    if (current_element != NULL) {
        if (raw_data.conn == 0x00) {    // connector id 0x00 has no timestamp, data starts at offset 4
            raw_data.data_size += 4;
            memcpy(raw_data.data, (uint8_t*)current_element + 4, raw_data.data_size);
        }
        else {
            memcpy(raw_data.data, (uint8_t*)current_element + 8, raw_data.data_size);
        }
    }
    return raw_data;
}

/** Get raw timeline data from the given raw data element. Timeline data uses the register id to encode the unix epoch time.
 *  Data bytes start directly after the "register id"; there is no further timestamp field. */
SpeedwireRawData SpeedwireInverterProtocol::getRawTimelineData(const void* const current_element, uint32_t element_length) const {
    uint32_t first_word = 0xffffffff;
    uint32_t second_word = 0xffffffff;
    if (current_element != NULL) {
        first_word  = SpeedwireByteEncoding::getUint32LittleEndian(current_element);
    }
    SpeedwireRawData raw_data = {
        first_word,                     // command      => set to unix epoch timestamp;
        first_word,                     // register id  => set to unix epoch timestamp;
        (uint8_t)(0x00),                // connector id => set to 0
        SpeedwireDataType::Unsigned64,  // type         => arbitrarily set to Unsigned64;
        first_word,                     // time         => set to unix epoch timestamp
        NULL,
        (element_length - 4 < sizeof(SpeedwireRawData::data) ? element_length - 4 : sizeof(SpeedwireRawData::data))
    };
    if (current_element != NULL) {
        memcpy(raw_data.data, (uint8_t*)current_element + 4, raw_data.data_size);
    }
    return raw_data;
}

/** Get a vector of all raw data elements given in this inverter packet */
std::vector<SpeedwireRawData> SpeedwireInverterProtocol::getRawDataElements(void) const {
    std::vector<SpeedwireRawData> elements;
    uint32_t element_length = getRawDataLength();
    if (element_length > 0) {
        const void* current_element = getFirstRawDataElement();
        while (current_element != NULL) {
            // if this is a COMMAND_YIELD or COMMAND_EVENT, there is no sub-structure, just inverter timestamp followed by 64-bit yield value
            if ((getCommandID() & 0xff000000) == 0x70000000) {
                SpeedwireRawData data = getRawTimelineData(current_element, element_length);
                elements.push_back(data);
            }
            else {
                SpeedwireRawData data = getRawData(current_element, element_length);
                elements.push_back(data);
                //fprintf(stdout, "%s\n", data.toString().c_str());
            }
            current_element = getNextRawDataElement(current_element, element_length);
        }
    }
    if (elements.size() != 0 && elements.size() != (getLastRegisterID() - getFirstRegisterID() + 1)) {
        fprintf(stdout, "missing register\n");
        //LocalHost::hexdump(udp, size);
    }
    return elements;
}

/** Print all raw data elements given in this inverter packet */
std::string SpeedwireInverterProtocol::toString(void) const {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
        "DstSusyID 0x%04x  DstSerialNumber 0x%06lx  DstControl 0x%04x  SrcSusyID 0x%04x  SrcSerialNumber 0x%06lx  SrcControl 0x%04x\n"
        "ErrorCode 0x%04x  FragmentCounter 0x%04x  PacketID 0x%04x  CommandID 0x%08lx  FirstRegisterID 0x%08lx  LastRegisterID 0x%08lx\n",
        getDstSusyID(), getDstSerialNumber(), getDstControl(), getSrcSusyID(), getSrcSerialNumber(), getSrcControl(),
        getErrorCode(), getFragmentCounter(), getPacketID(), getCommandID(), getFirstRegisterID(), getLastRegisterID());
    std::string result(buffer);

    //LocalHost::hexdump(udp + sma_data_offset, (size >= sma_data_offset ? size - sma_data_offset : 0));
    std::vector<SpeedwireRawData> elements = getRawDataElements();
    uint32_t register_id = getFirstRegisterID();
    for (const auto &el : elements) {
        snprintf(buffer, sizeof(buffer), "0x%08lx: %s\n", register_id, el.toString().c_str());
        result.append(std::string(buffer));
        register_id++;
    }
    return result;
}


/** Set destination susy id. */
void SpeedwireInverterProtocol::setDstSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_dst_susy_id_offset, value);
}

/** Set destination serial number. */
void SpeedwireInverterProtocol::setDstSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_dst_serial_number_offset, value);
}

/** Sset destination control word. */
void SpeedwireInverterProtocol::setDstControl(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_dst_control_offset, value);
}

/** Set source susy id */
void SpeedwireInverterProtocol::setSrcSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_src_susy_id_offset, value);
}

/** Set source serial number. */
void SpeedwireInverterProtocol::setSrcSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_src_serial_number_offset, value);
}

/** Set source control word. */
void SpeedwireInverterProtocol::setSrcControl(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_src_control_offset, value);
}

/** Set error code. */
void SpeedwireInverterProtocol::setErrorCode(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_error_code_offset, value);
}

/** Set fragment counter. */
void SpeedwireInverterProtocol::setFragmentCounter(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_fragment_counter_offset, value);
}

/** Set packet id. */
void SpeedwireInverterProtocol::setPacketID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_packet_id_offset, value);
}

/** Set command id. */
void SpeedwireInverterProtocol::setCommandID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_command_id_offset, value);
}

/** Set first register id. */
void SpeedwireInverterProtocol::setFirstRegisterID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_first_register_id_offset, value);
}

// set last register id. */
void SpeedwireInverterProtocol::setLastRegisterID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_last_register_id_offset, value);
}

/** Set 32-bit of data at the given offset in the data area. */
void SpeedwireInverterProtocol::setDataUint32(const unsigned long byte_offset, const uint32_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set 64-bit of data at the given offset in the data area. */
void SpeedwireInverterProtocol::setDataUint64(const unsigned long byte_offset, const uint64_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint64LittleEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set an array of 8-bit data at the given offset in the data area. */
void SpeedwireInverterProtocol::setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length) {
    memcpy(udp + sma_data_offset + byte_offset, value, value_length);
}

/** Set trailer long word at data offset. */
void SpeedwireInverterProtocol::setTrailer(const unsigned long offset) {
    setDataUint32(offset, 0x00000000);
}
