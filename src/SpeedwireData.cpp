#define  _CRT_SECURE_NO_WARNINGS (1)
#include <memory.h>
#include <time.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireData.hpp>
#include <SpeedwireCommand.hpp>
using namespace libspeedwire;


//! Convert SpeedwireDataType to a string
std::string libspeedwire::toString(SpeedwireDataType type) {
    switch (type) {
    case SpeedwireDataType::Unsigned32: return "Unsigned32";
    case SpeedwireDataType::Status32:   return "Status32";
    case SpeedwireDataType::String32:   return "String32";
    case SpeedwireDataType::Float:      return "Float";
    case SpeedwireDataType::Signed32:   return "Signed32";
    case SpeedwireDataType::Unsigned64: return "Unsigned64";
    case SpeedwireDataType::Event:      return "Event";
    case SpeedwireDataType::Yield:      return "Yield";
    }
    return "unknown-type";
}


/*******************************
 *  Class holding raw data from the speedwire inverter reply packet
 ********************************/
/**
 *  Constructor.
 *  @param _command The inverter/battéry command belonging to this raw data reply
 *  @param _id The register id
 *  @param _conn The connection number
 *  @param _type The type
 *  @param _time The packet time
 *  @param _data The binary data
 *  @param _data_size The size of the binary data
 */
SpeedwireRawData::SpeedwireRawData(const uint32_t _command, const uint32_t _id, const uint8_t _conn, const SpeedwireDataType _type, const time_t _time, const void *const _data, const size_t _data_size) :
    command(_command),
    id(_id),
    conn(_conn),
    type(_type),
    time(_time) {
    data_size = (_data_size < sizeof(data) ? _data_size : sizeof(data));
    memset(data, 0, sizeof(data));
    if (_data != NULL) {
        memcpy(data, _data, data_size);
    }
}

/**
 *  Default constructor; not very useful, but needed for std::map.
 */
SpeedwireRawData::SpeedwireRawData(void) :
    command(0), id(0), conn(0), type(SpeedwireDataType::Unsigned32), time(0), data_size(0) {
    memset(data, 0, sizeof(data));
}


/**
 *  Compare two instances of SpeedwireRawData with each other.
 *  @param other The SpeedwireRawData instance to compare with
 *  @return true if the both instances are identical, false otherwise
 */
bool SpeedwireRawData::equals(const SpeedwireRawData& other) const {
    return (command == other.command && id == other.id && conn == other.conn && type == other.type && time == other.time && data_size == other.data_size && memcmp(data, other.data, data_size) == 0);
}


/**
 *  Compare the signature of two SpeedwireRawData instances with each other.
 *  The signature consists of command, id, conn and type.
 *  @param other The SpeedwireRawData instance to compare with
 *  @return true if the signature is identical, false otherwise
 */
bool SpeedwireRawData::isSameSignature(const SpeedwireRawData& other) const {
    return (command == other.command && id == other.id && conn == other.conn && type == other.type);
}


/**
 *  Convert this instance into a std::string representation. Print data bytes as hex values.
 *  @return A string representation
 */
std::string SpeedwireRawData::toHexString(void) const {
    char buff[256];
    snprintf(buff, sizeof(buff), "id 0x%08lx conn 0x%02x type 0x%02x (%10s)  time 0x%08lx  data 0x", (unsigned)id, (unsigned)conn, (unsigned)type, libspeedwire::toString(type).c_str(), (uint32_t)time);
    std::string result(buff);
    for (size_t i = 0; i < data_size; ++i) {
        char byte[4];
        snprintf(byte, sizeof(byte), "%02x", (unsigned)data[i]);
        result.append(byte);
    }
    return result;
}


/**
 *  Convert this instance into a std::string representation. Interprete data bytes according to their type.
 *  @return A string representation
 */
std::string SpeedwireRawData::toString(void) const {
    // check if this raw data element is one of the predefined elements, if so get description string 
    std::string description = "unknown";
    const SpeedwireDataMap &data_map = SpeedwireDataMap::getGlobalMap();
    auto iterator = data_map.find(toKey());
    if (conn == 0x7 && iterator == data_map.end()) {
        // battery connection id is 0x7, however some definitions are identical to inverter connection id 0x1, check if this can be used
        iterator = data_map.find(toKey() ^ 0x6);
    }
    if (iterator != data_map.end()) {
        description = iterator->second.name;
        //unsigned long divisor = iterator->second.measurementType.divisor;
    }

    // assemble a string from the header fields
    char buff[256];
    snprintf(buff, sizeof(buff), "id 0x%08lx (%32s) conn 0x%02x type 0x%02x (%10s)  time 0x%08lx  data ", (unsigned)id, description.c_str(), (unsigned)conn, (unsigned)type, libspeedwire::toString(type).c_str(), (uint32_t)time);

    // extract raw values and append them to the string
    std::string result(buff);
    size_t num_values = getNumberOfValues();

    // for register data, append the binary representation of each data element, depending on its type
    if (conn != 0x00) {
        for (size_t i = 0; i < num_values; ++i) {
            std::string value_string;
            switch (type) {
            case SpeedwireDataType::Signed32: {
                SpeedwireRawDataSigned32 rd(*this);
                int32_t value = rd.getValue(i);
                value_string = rd.convertValueToString(value, true);
                break;
            }
            case SpeedwireDataType::Unsigned32: {
                SpeedwireRawDataUnsigned32 rd(*this);
                uint32_t value = rd.getValue(i);
                value_string = rd.convertValueToString(value, true);
                break;
            }
            case SpeedwireDataType::Status32: {
                SpeedwireRawDataStatus32 rd(*this);
                uint32_t value = rd.getValue(i);
                value_string = rd.convertValueToString(value);
                break;
            }
            //case SpeedwireDataType::Float: {
            //    float value = 0.0f;
            //    getFloatValue(i, value);
            //    snprintf(byte, sizeof(byte), " %7.2f", value);
            //    result.append(byte);
            //    break;
            //}
            case SpeedwireDataType::String32: {
                SpeedwireRawDataString32 rd(*this);
                value_string = rd.getHexValue(i);
                break;
            }
            case SpeedwireDataType::Unsigned64: {
                uint64_t value = SpeedwireByteEncoding::getUint64LittleEndian(data + i * sizeof(uint64_t));
                char str[32];
                snprintf(str, sizeof(str), "%016llx ", value);
                value_string.append(str);
            }
            }
            const size_t num_chars = 12;
            if (num_chars > value_string.length()) {
                result.append(num_chars - value_string.length(), ' ');
            }
            result.append(value_string);
        }
    }
    // for timeline data, append the binary representation of each data byte
    else {
        std::string value_string;
        for (size_t i = 0; i < data_size; ++i) {
            char byte[8];
            snprintf(byte, sizeof(byte), "%02x ", (unsigned int)data[i]);
            value_string.append(byte);
        }
        result.append(value_string);
    }

    // decode values and print their representation
    if (conn != 0x00) {
        switch (type) {
        case SpeedwireDataType::Signed32: {
            SpeedwireRawDataSigned32 rd(*this);
            std::vector<int32_t> values = rd.getValues();
            if (rd.isValueWithRange()) {
                result.append("  => ");
                result.append(rd.toValueWithRangeString());
            }
            else {
                for (size_t i = 0; i < values.size(); ++i) {
                    result.append((i == 0) ? "  => " : ", ");
                    result.append(rd.convertValueToString(values[i], false));
                }
            }
            break;
        }
        case SpeedwireDataType::Unsigned32: {
            SpeedwireRawDataUnsigned32 rd(*this);
            std::vector<uint32_t> values = rd.getValues();
            if (rd.isRevisionOrSerial()) {
                result.append("  => ");
                result.append(rd.toRevisionOrSerialString());
            }
            else if (rd.isValueWithRange()) {
                result.append("  => ");
                result.append(rd.toValueWithRangeString());
            }
            else {
                for (size_t i = 0; i < values.size(); ++i) {
                    result.append((i == 0) ? "  => " : ", ");
                    result.append(rd.convertValueToString(values[i], false));
                }
            }
            break;
        }
        case SpeedwireDataType::Status32: {
            SpeedwireRawDataStatus32 rd(*this);
            std::vector<uint32_t> values = rd.getValues();
            result.append("  => ");
            if (values.size() > 0) {
                result.append(rd.convertValueToString(values[0]));
            }
            break;
        }
        case SpeedwireDataType::String32: {
            SpeedwireRawDataString32 rd(*this);
            std::vector<std::string> values = rd.getValues();
            for (size_t i = 0; i < values.size(); ++i) {
                result.append((i == 0) ? "  => \"" : ", \"");
                result.append(rd.getValue(i).c_str());  // use c_str() to remove the 32 null bytes
                result.append("\"");
            }
            break;
        }
        case SpeedwireDataType::Unsigned64: {
            for (size_t i = 0; i < num_values; ++i) {
                result.append((i == 0) ? "  => " : ", ");
                uint64_t value = SpeedwireByteEncoding::getUint64LittleEndian(data + i * sizeof(uint64_t));
                char str[32];
                snprintf(str, sizeof(str), "%llu ", value);
                result.append(str);
            }
            break;
        }
        }
    }
    // decode timeline data and add formatted timestamp
    else {
        switch (type) {
        case SpeedwireDataType::Yield: {
            SpeedwireRawDataYield rd(*this);
            result.append("  => ");
            result.append(rd.convertValueToString(rd.getValue(0), false));
            break;
        }
        case SpeedwireDataType::Event: {
            SpeedwireRawDataEvent rd(*this);
            result.append("  => ");
            result.append(rd.convertValueToString(rd.getValue(0), false));
            break;
        }
        }
    }
    return result;
}


/** 
 *  Get number of data values available in the payload data.
 */
size_t SpeedwireRawData::getNumberOfValues(void) const {
    switch (type) {
    case SpeedwireDataType::Unsigned32:
        return data_size / SpeedwireRawDataUnsigned32::value_size;
    case SpeedwireDataType::Status32:
        return data_size / SpeedwireRawDataStatus32::value_size;
    case SpeedwireDataType::Float:
        return data_size / 4u;
    case SpeedwireDataType::Signed32:
        return data_size / SpeedwireRawDataSigned32::value_size;
    case SpeedwireDataType::String32:
        return data_size / SpeedwireRawDataString32::value_size;
    case SpeedwireDataType::Unsigned64:
        return data_size / 8u;
    case SpeedwireDataType::Yield:
        return data_size / SpeedwireRawDataYield::value_size;
    case SpeedwireDataType::Event:
        return data_size / SpeedwireRawDataEvent::value_size;
    }
    return 0;
}


/**
 * Determine the number of significant data values available in the payload data.
 *
 * For types Unsigned32 and Signed32 the following cases have been seen in packets:
 * - 2 values, the last one is 0
 *   => the first value is significant; this is used to encode history data, like daily consumption, etc.
 * - 5 values, the last one is 1, the first four values are identical
 *   => there is just one significant value; this is used to encode measurement values
 * - 5 values, the last one is 1, the first three values are different, the third and fourth values are identical
 *   => the first three values are significant; this is used to encode measurement values
 * - 8 values, pairs of values are identical
 *   => the four pairs are significant to encode a settings data values with range: min_value, max_value, value, unknown
 */
size_t SpeedwireRawData::getNumberOfSignificantValues(void) const {
    if (type == SpeedwireDataType::Unsigned32 || type == SpeedwireDataType::Signed32) {
        size_t num_values = getNumberOfValues();
        if (num_values == 2) {
            uint32_t value1 = SpeedwireByteEncoding::getUint32LittleEndian(data);
            uint32_t value2 = SpeedwireByteEncoding::getUint32LittleEndian(data + 4u);
            return (value2 == 0 ? 1 : 2);
        }
        else if (num_values == 5) {
            uint32_t value1 = SpeedwireByteEncoding::getUint32LittleEndian(data);
            uint32_t value2 = SpeedwireByteEncoding::getUint32LittleEndian(data + 4u);
            uint32_t value3 = SpeedwireByteEncoding::getUint32LittleEndian(data + 2 * 4u);
            uint32_t value4 = SpeedwireByteEncoding::getUint32LittleEndian(data + 3 * 4u);
            uint32_t value5 = SpeedwireByteEncoding::getUint32LittleEndian(data + 4 * 4u);
            if (value5 == 1) {
                if (value1 == value2 && value2 == value3 && value3 == value4) {
                    return 1;
                }
                else if (value3 == value4) {
                    return 3;
                }
                return 4;
            }
            return 5;
        }
        else if (num_values == 8) {
            uint32_t value1 = SpeedwireByteEncoding::getUint32LittleEndian(data);
            uint32_t value2 = SpeedwireByteEncoding::getUint32LittleEndian(data + 4u);
            uint32_t value3 = SpeedwireByteEncoding::getUint32LittleEndian(data + 2 * 4u);
            uint32_t value4 = SpeedwireByteEncoding::getUint32LittleEndian(data + 3 * 4u);
            uint32_t value5 = SpeedwireByteEncoding::getUint32LittleEndian(data + 4 * 4u);
            uint32_t value6 = SpeedwireByteEncoding::getUint32LittleEndian(data + 5 * 4u);
            uint32_t value7 = SpeedwireByteEncoding::getUint32LittleEndian(data + 6 * 4u);
            uint32_t value8 = SpeedwireByteEncoding::getUint32LittleEndian(data + 7 * 4u);
            if (value1 == value2 && value3 == value4 && value5 == value6 && value7 == value8) {
                return 4;
            }
        }
    }
    printf("unexpected raw data value sequence\n");
    return getNumberOfValues();
}


/**
 * Decode the sequence of raw data values into a vector of significant values.
 *
 * For types Unsigned32 and Signed32 the following cases have been seen in packets:
 * - 2 values, the last one is 0
 *   => the first value is significant; this is used to encode history data, like daily consumption, etc.
 * - 5 values, the last one is 1, the first four values are identical
 *   => there is just one significant value; this is used to encode measurement values
 * - 5 values, the last one is 1, the first three values are different, the third and fourth values are identical
 *   => the first three values are significant; this is used to encode measurement values
 * - 8 values, pairs of values are identical
 *   => the four pairs are significant to encode a settings data values with range: min_value, max_value, value, unknown
 */
std::vector<uint32_t> SpeedwireRawDataUnsigned32::getValues(void) const {
    std::vector<uint32_t> values;
    size_t n = base.getNumberOfSignificantValues();
    size_t d = (getNumberOfValues() == 8 ? 2 : 1);
    for (size_t i = 0; i < n; ++i) {
        values.push_back(getValue(i * d));
    }
    return values;
}


/**
 * Convert the sequence of raw data values into a string, provided it is a value with range (i.e. 4 pairs of values).
 * @return a string representation or an empty string if is not a value with range
 */
std::string  SpeedwireRawDataUnsigned32::toValueWithRangeString(void) const {
    std::string result;
    if (isValueWithRange()) {
        auto values = getValues();
        result.append("(");
        result.append(convertValueToString(values[0], false)); result.append("...");
        result.append(convertValueToString(values[1], false)); result.append(") ");
        result.append(convertValueToString(values[2], false));
    }
    return result;
}


static bool isBCD(uint8_t value) {
    return ((value & 0xf0) >= 0x00 && (value & 0xf0) <= 0x90 && (value & 0x0f) >= 0x00 && (value & 0x0f) <= 0x09);
}

/**
 * Convert the sequence of raw data values into a string, provided it is a value with range (i.e. 4 pairs of values) encoding a revision or serial number.
 * @return a string representation or an empty string if is not a revision or serial number
 */
std::string  SpeedwireRawDataUnsigned32::toRevisionOrSerialString(void) const {
    std::string result;
    if (isRevisionOrSerial()) {
        uint32_t value = getValue(4);  // values 4 and 5 hold the revision or serial
        uint8_t byte0 = (value >> 24) & 0xff;
        uint8_t byte1 = (value >> 16) & 0xff;
        uint8_t byte2 = (value >>  8) & 0xff;
        uint8_t byte3 = value & 0xff;
        if (isBCD(byte0) && isBCD(byte1) && byte3 <= 5) {
            result.append("revision ");
            result.append(convertValueToString((byte0 >> 4) & 0xf, false)); result.append(convertValueToString(byte0 & 0xf, false)); result.append(".");
            result.append(convertValueToString((byte1 >> 4) & 0xf, false)); result.append(convertValueToString(byte1 & 0xf, false)); result.append(".");
            result.append(convertValueToString(byte2, false)); result.append(".");  // binary encoded
            switch (byte3) {
            case 0: result.append("N"); break;
            case 1: result.append("E"); break;
            case 2: result.append("A"); break;
            case 3: result.append("B"); break;
            case 4: result.append("R"); break;
            case 5: result.append("S"); break;
            default: break;
            }
        }
        else if (value >= 1000000000) {
            result.append("serial ");
            result.append(convertValueToString(value, false));
        }
        else {
            result.append(toValueWithRangeString());
        }
    }
    return result;
}


/**
 * Decode the sequence of raw data values into a vector of significant values.
 *
 * For types Unsigned32 and Signed32 the following cases have been seen in packets:
 * - 2 values, the last one is 0
 *   => the first value is significant; this is used to encode history data, like daily consumption, etc.
 * - 5 values, the last one is 1, the first four values are identical
 *   => there is just one significant value; this is used to encode measurement values
 * - 5 values, the last one is 1, the first three values are different, the third and fourth values are identical
 *   => the first three values are significant; this is used to encode measurement values
 * - 8 values, pairs of values are identical
 *   => the four pairs are significant to encode a settings data values with range: min_value, max_value, value, unknown
 */
std::vector<int32_t> SpeedwireRawDataSigned32::getValues(void) const {
    std::vector<int32_t> values;
    size_t n = base.getNumberOfSignificantValues();
    size_t d = (getNumberOfValues() == 8 ? 2 : 1);
    for (size_t i = 0; i < n; ++i) {
        values.push_back(getValue(i * d));
    }
    return values;

}


/**
 * Convert the sequence of raw data values into a string, provided it is a value with range (i.e. 4 pairs of values).
 * @return a string representation or an empty string if is not a value with range
 */
std::string  SpeedwireRawDataSigned32::toValueWithRangeString(void) const {
    std::string result;
    if (isValueWithRange()) {
        auto values = getValues();
        result.append("(");
        result.append(convertValueToString(values[0], false)); result.append("...");
        result.append(convertValueToString(values[1], false)); result.append(") ");
        result.append(convertValueToString(values[2], false));
    }
    return result;
}


/**
 * Constructor for EventValues. It parses the content from the byte array.
 */
SpeedwireRawDataEvent::EventValue::EventValue(time_t time, uint8_t* data, size_t data_size) {
    epoch_time = time;

    SpeedwireRawData rd(0x00000000, 0, 0, SpeedwireDataType::Unsigned32, time, data, data_size);
    SpeedwireRawDataUnsigned32 rd32(rd);
    counter       = rd32.getValue(0) & 0x0000ffff;      // some counter
    susy_id       = (rd32.getValue(0) >> 16) & 0xffff;  // susy id
    serial_number = rd32.getValue(1);                   // serial number
    event_id      = rd32.getValue(2) & 0x0000ffff;      // event id
    marker_1      = (rd32.getValue(2) >> 16) & 0xff;    // unknown marker 1
    marker_2      = (rd32.getValue(2) >> 24) & 0xff;    // unknown marker 2
    for (size_t i = 0; i < 7; ++i) {
        value[i]  = rd32.getValue(3 + i);               // unknown 32-bit value
    }
}


/**
 * Convert the sequence of raw data event values into a string.
 * @return a string representation
 */
std::string SpeedwireRawDataEvent::convertValueToString(const EventValue& value, bool hex) const {
    std::string result = LocalHost::unixEpochTimeInMsToString(SpeedwireTime::convertInverterTimeToUnixEpochTime((uint32_t)value.epoch_time));
    result.append(" ");

    SpeedwireRawDataUnsigned32 rd(base);
    for (size_t i = 0; i < base.getNumberOfValues(); ++i) {
        EventValue value = getValue(i);
        result.append(rd.convertValueToString(value.counter, false));
        result.append(", ");
        result.append(rd.convertValueToString(value.susy_id, false));
        result.append("-");
        result.append(rd.convertValueToString(value.serial_number, false));
        result.append(", ");
        result.append(rd.convertValueToString(value.event_id, false));
        result.append("-");
        result.append(rd.convertValueToString(value.marker_1, false));
        result.append("-");
        result.append(rd.convertValueToString(value.marker_2, false));
        for (size_t j = 0; j < 7; ++j) {
            result.append(", ");
            result.append(rd.convertValueToString(value.value[j], false));
        }
    }
    return result;
}



/*******************************
 *  Class holding data from the speedwire inverter reply packet, enriched by measurement type information
 *  and the interpreted measurement value
 ********************************/

/**
 *  Constructor
 *  @param command The inverter/battéry command belonging to this raw data reply
 *  @param id The register id
 *  @param conn The connection number
 *  @param type The type
 *  @param time The packet time
 *  @param data The binary data
 *  @param data_size The size of the binary data
 *  @param mType The MeasurementType
 *  @param _wire The Wire
 */
SpeedwireData::SpeedwireData(const uint32_t command, const uint32_t id, const uint8_t conn, const SpeedwireDataType type, const time_t time, const void *const data, const size_t data_size,
                             const MeasurementType& mType, const Wire _wire, const std::string &sname) :
    SpeedwireRawData(command, id, conn, type, time, data, data_size),
    Measurement(mType, _wire),
    name(sname) {
}


/**
 *  Default constructor; not very useful, but needed for std::map.
 */
SpeedwireData::SpeedwireData(void) :
    SpeedwireRawData(0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0),
    Measurement(MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::NO_QUANTITY, "", 0), Wire::NO_WIRE) {
    description = "";
}


/**
 *  Consume the value and timer of the given inverter raw data into this instance.
 *  This is done by interpreting the register id and converting numeric values to values in physical quantities
 *  before taking them into this instance.
 *  @param data The SpeedwireRawData instance to be consumed into this instance
 *  @result true if the data was successfuly consumed, false otherwise
 */
bool SpeedwireData::consume(const SpeedwireRawData& data) {
    if (!isSameSignature(data)) return false;
    if (data.data == NULL || data.data_size < 20) return false;

    switch (type) {

    case SpeedwireDataType::Signed32: {
        SpeedwireRawDataSigned32 rd((SpeedwireRawData&)data);
        int32_t value = rd.getValue(0);
        if (rd.isNanValue(value)) value = 0;  // received during darkness: NaN value is 0x80000000
#if 0   // simulate some values for debugging
        if (id == 0x00251e00) value = 0x57;
        if (id == 0x00451f00) value = 0x6105;
        if (id == 0x00452100) value = 0x0160;
        if (id == 0x00464000 || id == 0x00464100 || id == 0x00464200) value = 0x0038;
        if (id == 0x00464800 || id == 0x00464900 || id == 0x00464a00) value = 0x59cf;
        if (id == 0x00464b00 || id == 0x00464c00 || id == 0x00464d00) value = 0x9b3c;
        if (id == 0x00465300 || id == 0x00465400 || id == 0x00465500) value = 0x011e;
#endif
        addMeasurement(value, (uint32_t)data.time);
        time = data.time;
        break;
    }

    case SpeedwireDataType::Unsigned32: {
        SpeedwireRawDataUnsigned32 rd((SpeedwireRawData&)data);
        int32_t value = rd.getValue(0);
        if (rd.isNanValue(value)) value = 0;  // received during darkness: NaN value is 0xffffffff
        addMeasurement(value, (uint32_t)data.time);
        time = data.time;
        break;
    }

    case SpeedwireDataType::Status32: {
        SpeedwireRawDataStatus32 rd((SpeedwireRawData &)data);
        switch (id) {
        case 0x00214800: {   // device status
            bool ok = false;
            size_t index = rd.getSelectionIndex();
            if (index != (size_t)-1) {
                uint32_t value = rd.getValue(index);
                ok = ((value & 0x00ffffff) == 0x133);  // 307 <=> OK (from: Technische Beschreibung SC - COM Modbus® - Schnittstelle)
            }
            addMeasurement(ok, (uint32_t)data.time);
            break;
        }
        case 0x00416400: {  // grid relay status
            // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000980 00028051 00482100 ff482100 00000000 =>  query device status
            // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000980 01028051 00000000 00000000 01482108 59c5e95f 33010001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000 00000000
            // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000a80 00028051 00644100 ff644100 00000000 =>  query grid relay status
            // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000a80 01028051 07000000 07000000 01644108 59c5e95f 33000001 37010000 fdffff00 feffff00 00000000 00000000 00000000 00000000 00000000
            // the inverter replies with a list of 3 status value: 0x33 (on), 0x137 (open) 0x00fffffd (NaN) (from: Technische Beschreibung SC-COM Modbus®-Schnittstelle)
            // one of the value has a 0x01000000 marker; this is the one that is valid
            bool on = false;
            size_t index = rd.getSelectionIndex();
            if (index != (size_t)-1) {
                uint32_t value = rd.getValue(index);
                on = ((value & 0x00ffffff) == 0x000033);
            }
            addMeasurement(on, (uint32_t)data.time);
            time = data.time;
            break;
        }
        default:
            perror("unsupported id");
            return false;
        }
        break;
    }
    default:
        perror("unsupported SpeedwireDataType");
        return false;
    }

    return true;
}


/**
 *  Convert this instance into a std::string representation.
 *  @return A string representation
 */
std::string SpeedwireData::toString(void) const {
    TimestampDoublePair measurementValue = measurementValues.getNewestElement();
    char buff[256];
    snprintf(buff, sizeof(buff), "%-16s  time %lu  %s  => %lf %s\n", description.c_str(), measurementValue.time, SpeedwireRawData::toString().c_str(), measurementValue.value, measurementType.unit.c_str());
    return std::string(buff);
}


/**
 *  Get a vector of all pre - defined SpeedwireData instances.
 */
std::vector<SpeedwireData> SpeedwireData::getAllPredefined(void) {
    std::vector<SpeedwireData> predefined;

    predefined.push_back(InverterDiscovery);
    predefined.push_back(InverterDeviceName);
    predefined.push_back(InverterDeviceClass);
    predefined.push_back(InverterDeviceType);
    predefined.push_back(InverterSoftwareVersion);
    predefined.push_back(InverterPowerMPP1);
    predefined.push_back(InverterPowerMPP2);
    predefined.push_back(InverterVoltageMPP1);
    predefined.push_back(InverterVoltageMPP2);
    predefined.push_back(InverterCurrentMPP1);
    predefined.push_back(InverterCurrentMPP2);
    predefined.push_back(InverterPowerL1);
    predefined.push_back(InverterPowerL2);
    predefined.push_back(InverterPowerL3);
    predefined.push_back(InverterVoltageL1);
    predefined.push_back(InverterVoltageL2);
    predefined.push_back(InverterVoltageL3);
    predefined.push_back(InverterVoltageL1toL2);
    predefined.push_back(InverterVoltageL2toL3);
    predefined.push_back(InverterVoltageL3toL1);
    predefined.push_back(InverterPowerFactor);
    predefined.push_back(InverterCurrentL1);
    predefined.push_back(InverterCurrentL2);
    predefined.push_back(InverterCurrentL3);
    predefined.push_back(InverterFrequency);
    predefined.push_back(InverterPowerACTotal);
    predefined.push_back(InverterReactivePowerTotal);
    predefined.push_back(InverterNominalPower);
    predefined.push_back(InverterEnergyTotal);
    predefined.push_back(InverterEnergyDaily);
    predefined.push_back(InverterGridExportEnergyTotal);
    predefined.push_back(InverterGridImportEnergyTotal);
    predefined.push_back(InverterOperationTime);
    predefined.push_back(InverterFeedInTime);
    predefined.push_back(InverterOperationStatus);
    predefined.push_back(InverterUpdateStatus);
    predefined.push_back(InverterMessageStatus);
    predefined.push_back(InverterActionStatus);
    predefined.push_back(InverterDescriptionStatus);
    predefined.push_back(InverterErrorStatus);
    predefined.push_back(InverterRelay);

    predefined.push_back(BatterySoftwareVersion);
    predefined.push_back(BatteryStateOfCharge);
    predefined.push_back(BatteryDiagChargeCycles);
    predefined.push_back(BatteryDiagTotalAhIn);
    predefined.push_back(BatteryDiagTotalAhOut);
    predefined.push_back(BatteryTemperature);
    predefined.push_back(BatteryVoltage);
    predefined.push_back(BatteryCurrent);
    predefined.push_back(BatteryPowerL1);
    predefined.push_back(BatteryPowerL2);
    predefined.push_back(BatteryPowerL3);
    predefined.push_back(BatteryVoltageL1);
    predefined.push_back(BatteryVoltageL2);
    predefined.push_back(BatteryVoltageL3);
    predefined.push_back(BatteryVoltageL1toL2);
    predefined.push_back(BatteryVoltageL2toL3);
    predefined.push_back(BatteryVoltageL3toL1);
    predefined.push_back(BatteryCurrentL1);
    predefined.push_back(BatteryCurrentL2);
    predefined.push_back(BatteryCurrentL3);
    predefined.push_back(BatteryGridVoltageL1);
    predefined.push_back(BatteryGridVoltageL2);
    predefined.push_back(BatteryGridVoltageL3);
    predefined.push_back(BatteryGridPositivePowerL1);
    predefined.push_back(BatteryGridPositivePowerL2);
    predefined.push_back(BatteryGridPositivePowerL3);
    predefined.push_back(BatteryGridNegativePowerL1);
    predefined.push_back(BatteryGridNegativePowerL2);
    predefined.push_back(BatteryGridNegativePowerL3);
    predefined.push_back(BatteryGridReactivePowerL1);
    predefined.push_back(BatteryGridReactivePowerL2);
    predefined.push_back(BatteryGridReactivePowerL3);
    predefined.push_back(BatteryGridReactivePower);
    predefined.push_back(BatterySetVoltage);

    predefined.push_back(InverterPowerDCTotal);
    predefined.push_back(InverterPowerLoss);
    predefined.push_back(InverterPowerEfficiency);
    predefined.push_back(BatteryPowerACTotal);

    predefined.push_back(HouseholdPowerTotal);
    predefined.push_back(HouseholdIncomeTotal);
    predefined.push_back(HouseholdIncomeFeedIn);
    predefined.push_back(HouseholdIncomeSelfConsumption);

    predefined.push_back(YieldByMinute);
    predefined.push_back(YieldByDay);
    predefined.push_back(Event);

    return predefined;
}


// pre-defined SpeedwireData instances
const SpeedwireData SpeedwireData::InverterDiscovery         (Command::COMMAND_DEVICE_QUERY, 0x00000300, 0x00, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "Discovery");
const SpeedwireData SpeedwireData::InverterDeviceName        (Command::COMMAND_DEVICE_QUERY, 0x00821e00, 0x01, SpeedwireDataType::String32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "Name");
const SpeedwireData SpeedwireData::InverterDeviceClass       (Command::COMMAND_DEVICE_QUERY, 0x00821f00, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "MainModel");
const SpeedwireData SpeedwireData::InverterDeviceType        (Command::COMMAND_DEVICE_QUERY, 0x00822000, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "Model");
const SpeedwireData SpeedwireData::InverterSoftwareVersion   (Command::COMMAND_DEVICE_QUERY, 0x00823400, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "SwRev");
const SpeedwireData SpeedwireData::InverterPowerMPP1         (Command::COMMAND_DC_QUERY,     0x00251E00, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::MPP1, "PpvdcA");
const SpeedwireData SpeedwireData::InverterPowerMPP2         (Command::COMMAND_DC_QUERY,     0x00251E00, 0x02, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::MPP2, "PpvdcB");
const SpeedwireData SpeedwireData::InverterVoltageMPP1       (Command::COMMAND_DC_QUERY,     0x00451F00, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::MPP1, "UpvdcA");
const SpeedwireData SpeedwireData::InverterVoltageMPP2       (Command::COMMAND_DC_QUERY,     0x00451F00, 0x02, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::MPP2, "UpvdcB");
const SpeedwireData SpeedwireData::InverterCurrentMPP1       (Command::COMMAND_DC_QUERY,     0x00452100, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::MPP1, "IpvdcA");
const SpeedwireData SpeedwireData::InverterCurrentMPP2       (Command::COMMAND_DC_QUERY,     0x00452100, 0x02, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::MPP2, "IpvdcB");
const SpeedwireData SpeedwireData::InverterPowerL1           (Command::COMMAND_AC_QUERY,     0x00464000, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L1, "PacL1");
const SpeedwireData SpeedwireData::InverterPowerL2           (Command::COMMAND_AC_QUERY,     0x00464100, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L2, "PacL2");
const SpeedwireData SpeedwireData::InverterPowerL3           (Command::COMMAND_AC_QUERY,     0x00464200, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L3, "PacL3");
const SpeedwireData SpeedwireData::InverterVoltageL1         (Command::COMMAND_AC_QUERY,     0x00464800, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1, "UacL1");    // L1 -> N
const SpeedwireData SpeedwireData::InverterVoltageL2         (Command::COMMAND_AC_QUERY,     0x00464900, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2, "UacL2");    // L2 -> N
const SpeedwireData SpeedwireData::InverterVoltageL3         (Command::COMMAND_AC_QUERY,     0x00464a00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3, "UacL3");    // L3 -> N
const SpeedwireData SpeedwireData::InverterVoltageL1toL2     (Command::COMMAND_AC_QUERY,     0x00464b00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1L2, "UacL1L2");  // L1 -> L2
const SpeedwireData SpeedwireData::InverterVoltageL2toL3     (Command::COMMAND_AC_QUERY,     0x00464c00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2L3, "UacL2L3");  // L2 -> L3
const SpeedwireData SpeedwireData::InverterVoltageL3toL1     (Command::COMMAND_AC_QUERY,     0x00464d00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3L1, "UacL3L1");  // L3 -> L1
const SpeedwireData SpeedwireData::InverterPowerFactor       (Command::COMMAND_AC_QUERY,     0x00464e00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPowerFactor(), Wire::TOTAL, "PacCosPhi");
const SpeedwireData SpeedwireData::InverterCurrentL1         (Command::COMMAND_AC_QUERY,     0x00465300, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L1, "IacL1");
const SpeedwireData SpeedwireData::InverterCurrentL2         (Command::COMMAND_AC_QUERY,     0x00465400, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L2, "IacL2");
const SpeedwireData SpeedwireData::InverterCurrentL3         (Command::COMMAND_AC_QUERY,     0x00465500, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L3, "IacL3");
const SpeedwireData SpeedwireData::InverterFrequency         (Command::COMMAND_AC_QUERY,     0x00465700, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterFrequency(), Wire::TOTAL, "Fac");
const SpeedwireData SpeedwireData::InverterPowerACTotal      (Command::COMMAND_AC_QUERY,     0x00263f00, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::TOTAL, "Pac");
const SpeedwireData SpeedwireData::InverterReactivePowerTotal(Command::COMMAND_AC_QUERY,     0x00265f00, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterReactivePower(), Wire::TOTAL, "Qac");
const SpeedwireData SpeedwireData::InverterNominalPower      (Command::COMMAND_AC_QUERY,     0x00411e00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterNominalPower(), Wire::TOTAL, "Pnominal");
const SpeedwireData SpeedwireData::InverterEnergyTotal          (Command::COMMAND_ENERGY_QUERY, 0x00260100, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterEnergy(), Wire::TOTAL, "Etotal");
const SpeedwireData SpeedwireData::InverterEnergyDaily          (Command::COMMAND_ENERGY_QUERY, 0x00262200, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterEnergy(), Wire::NO_WIRE, "Edaily");
const SpeedwireData SpeedwireData::InverterGridExportEnergyTotal(Command::COMMAND_ENERGY_QUERY, 0x00462400, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterEnergy(Direction::NEGATIVE), Wire::GRID_TOTAL, "Eexport");
const SpeedwireData SpeedwireData::InverterGridImportEnergyTotal(Command::COMMAND_ENERGY_QUERY, 0x00462500, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterEnergy(Direction::POSITIVE), Wire::GRID_TOTAL, "Eimport");
const SpeedwireData SpeedwireData::InverterOperationTime        (Command::COMMAND_ENERGY_QUERY, 0x00462e00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterDuration(), Wire::TOTAL, "htotal");
const SpeedwireData SpeedwireData::InverterFeedInTime           (Command::COMMAND_ENERGY_QUERY, 0x00462f00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterDuration(), Wire::NO_WIRE, "hon");
const SpeedwireData SpeedwireData::InverterOperationStatus      (Command::COMMAND_STATUS_QUERY, 0x00214800, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(),  Wire::DEVICE_OK, "OpInvCtlStt");
const SpeedwireData SpeedwireData::InverterUpdateStatus         (Command::COMMAND_STATUS_QUERY, 0x00412900, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "OpInvUpdStt");
const SpeedwireData SpeedwireData::InverterMessageStatus        (Command::COMMAND_STATUS_QUERY, 0x00414900, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "OpInvMsgStt");
const SpeedwireData SpeedwireData::InverterActionStatus         (Command::COMMAND_STATUS_QUERY, 0x00414a00, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "OpInvActnStt");
const SpeedwireData SpeedwireData::InverterDescriptionStatus    (Command::COMMAND_STATUS_QUERY, 0x00414b00, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "OpInvDscrStt");
const SpeedwireData SpeedwireData::InverterErrorStatus          (Command::COMMAND_STATUS_QUERY, 0x00414c00, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "OpInvErrStt");
const SpeedwireData SpeedwireData::InverterRelay                (Command::COMMAND_STATUS_QUERY, 0x00416400, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterRelay(),   Wire::RELAY_ON, "OpGriSwStt");

const SpeedwireData SpeedwireData::BatterySoftwareVersion    (Command::COMMAND_AC_QUERY, 0x00823300, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "SwRev");
const SpeedwireData SpeedwireData::BatteryPowerACTotal       (Command::COMMAND_AC_QUERY, 0x00263F00, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::TOTAL, "BatPacTotal");
const SpeedwireData SpeedwireData::BatteryStateOfCharge      (Command::COMMAND_AC_QUERY, 0x00295a00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterStateOfCharge(), Wire::NO_WIRE, "BatSoC");
const SpeedwireData SpeedwireData::BatteryDiagChargeCycles   (Command::COMMAND_AC_QUERY, 0x00491e00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterRelay(), Wire::NO_WIRE, "BatChargeCycl");
const SpeedwireData SpeedwireData::BatteryDiagTotalAhIn      (Command::COMMAND_AC_QUERY, 0x00492600, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterRelay(), Wire::NO_WIRE, "BatTotAhIn");
const SpeedwireData SpeedwireData::BatteryDiagTotalAhOut     (Command::COMMAND_AC_QUERY, 0x00492700, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterRelay(), Wire::NO_WIRE, "BatTotAhOut");
const SpeedwireData SpeedwireData::BatteryTemperature        (Command::COMMAND_AC_QUERY, 0x00495b00, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterTemperature(), Wire::NO_WIRE, "BatTemp");
const SpeedwireData SpeedwireData::BatteryVoltage            (Command::COMMAND_AC_QUERY, 0x00495c00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::NO_WIRE, "BatUdc");
const SpeedwireData SpeedwireData::BatteryCurrent            (Command::COMMAND_AC_QUERY, 0x00495d00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::NO_WIRE, "BatIdc");
const SpeedwireData SpeedwireData::BatteryPowerL1            (Command::COMMAND_AC_QUERY, 0x00464000, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::L1, "BatPacL1");
const SpeedwireData SpeedwireData::BatteryPowerL2            (Command::COMMAND_AC_QUERY, 0x00464100, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::L2, "BatPacL2");
const SpeedwireData SpeedwireData::BatteryPowerL3            (Command::COMMAND_AC_QUERY, 0x00464200, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::L3, "BatPacL3");
const SpeedwireData SpeedwireData::BatteryVoltageL1          (Command::COMMAND_AC_QUERY, 0x00464800, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1, "BatUacL1");    // L1 -> N);
const SpeedwireData SpeedwireData::BatteryVoltageL2          (Command::COMMAND_AC_QUERY, 0x00464900, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2, "BatUacL2");    // L2 -> N);
const SpeedwireData SpeedwireData::BatteryVoltageL3          (Command::COMMAND_AC_QUERY, 0x00464a00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3, "BatUacL3");    // L3 -> N);
const SpeedwireData SpeedwireData::BatteryVoltageL1toL2      (Command::COMMAND_AC_QUERY, 0x00464b00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1L2, "BatUacL1L2");  // L1 -> L2);
const SpeedwireData SpeedwireData::BatteryVoltageL2toL3      (Command::COMMAND_AC_QUERY, 0x00464c00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2L3, "BatUacL2L3");  // L2 -> L3);
const SpeedwireData SpeedwireData::BatteryVoltageL3toL1      (Command::COMMAND_AC_QUERY, 0x00464d00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3L1, "BatUacL3L1");  // L3 -> L1);
const SpeedwireData SpeedwireData::BatteryCurrentL1          (Command::COMMAND_AC_QUERY, 0x00465300, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L1, "BatIacL1");
const SpeedwireData SpeedwireData::BatteryCurrentL2          (Command::COMMAND_AC_QUERY, 0x00465400, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L2, "BatIacL2");
const SpeedwireData SpeedwireData::BatteryCurrentL3          (Command::COMMAND_AC_QUERY, 0x00465500, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L3, "BatIacL3");
const SpeedwireData SpeedwireData::BatteryGridVoltageL1      (Command::COMMAND_AC_QUERY, 0x0046e500, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::NO_WIRE, "GridUacL1");
const SpeedwireData SpeedwireData::BatteryGridVoltageL2      (Command::COMMAND_AC_QUERY, 0x0046e600, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::NO_WIRE, "GridUacL2");
const SpeedwireData SpeedwireData::BatteryGridVoltageL3      (Command::COMMAND_AC_QUERY, 0x0046e700, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::NO_WIRE, "GridUacL3");
const SpeedwireData SpeedwireData::BatteryGridPositivePowerL1(Command::COMMAND_AC_QUERY, 0x0046e800, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::NO_WIRE, "GridPosPacL1");
const SpeedwireData SpeedwireData::BatteryGridPositivePowerL2(Command::COMMAND_AC_QUERY, 0x0046e900, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::NO_WIRE, "GridPosPacL2");
const SpeedwireData SpeedwireData::BatteryGridPositivePowerL3(Command::COMMAND_AC_QUERY, 0x0046ea00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::NO_WIRE, "GridPosPacL3");
const SpeedwireData SpeedwireData::BatteryGridNegativePowerL1(Command::COMMAND_AC_QUERY, 0x0046eb00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::NO_WIRE, "GridNegPacL1");
const SpeedwireData SpeedwireData::BatteryGridNegativePowerL2(Command::COMMAND_AC_QUERY, 0x0046ec00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::NO_WIRE, "GridNegPacL2");
const SpeedwireData SpeedwireData::BatteryGridNegativePowerL3(Command::COMMAND_AC_QUERY, 0x0046ed00, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::NO_WIRE, "GridNegPacL3");
const SpeedwireData SpeedwireData::BatteryGridReactivePowerL1(Command::COMMAND_AC_QUERY, 0x0046ee00, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterReactivePower(), Wire::NO_WIRE, "GridQacL1");
const SpeedwireData SpeedwireData::BatteryGridReactivePowerL2(Command::COMMAND_AC_QUERY, 0x0046ef00, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterReactivePower(), Wire::NO_WIRE, "GridQacL2");
const SpeedwireData SpeedwireData::BatteryGridReactivePowerL3(Command::COMMAND_AC_QUERY, 0x0046f000, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterReactivePower(), Wire::NO_WIRE, "GridQacL3");
const SpeedwireData SpeedwireData::BatteryGridReactivePower  (Command::COMMAND_AC_QUERY, 0x0046f100, 0x07, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterReactivePower(), Wire::NO_WIRE, "GridQac");
const SpeedwireData SpeedwireData::BatterySetVoltage         (Command::COMMAND_AC_QUERY, 0x00493300, 0x07, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::NO_WIRE, "BatSetUdc");
const SpeedwireData SpeedwireData::BatteryOperationStatus    (Command::COMMAND_STATUS_QUERY, 0x00214800, 0x07, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::DEVICE_OK, "OpInvCtlStt");
const SpeedwireData SpeedwireData::BatteryRelay              (Command::COMMAND_STATUS_QUERY, 0x00416400, 0x07, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterRelay(),   Wire::RELAY_ON, "OpGriSwStt");
const SpeedwireData SpeedwireData::BatteryType               (Command::COMMAND_STATUS_QUERY, 0x00918d00, 0x07, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "BmsType");

// pre-defined instances of derived measurement values
const SpeedwireData SpeedwireData::InverterPowerDCTotal   (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(),      Wire::MPP_TOTAL, "Pdc");
const SpeedwireData SpeedwireData::InverterPowerLoss      (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterLoss(),       Wire::LOSS_TOTAL, "Ploss");
const SpeedwireData SpeedwireData::InverterPowerEfficiency(0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterEfficiency(), Wire::NO_WIRE, "Peff");

// pre-defined instances for miscellaneous measurement types
const SpeedwireData SpeedwireData::HouseholdPowerTotal           (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::TOTAL, "Phh");
const SpeedwireData SpeedwireData::HouseholdIncomeTotal          (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::Currency(),      Wire::TOTAL, "Chh");
const SpeedwireData SpeedwireData::HouseholdIncomeFeedIn         (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::Currency(),      Wire::FEED_IN, "ChhFeedIn");
const SpeedwireData SpeedwireData::HouseholdIncomeSelfConsumption(0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::Currency(),      Wire::SELF_CONSUMPTION, "ChhCons");

const SpeedwireData SpeedwireData::YieldByMinute                 (Command::COMMAND_YIELD_BY_MINUTE_QUERY, Command::COMMAND_YIELD_BY_MINUTE_QUERY, 0, SpeedwireDataType::Yield, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "EYieldByMinute");
const SpeedwireData SpeedwireData::YieldByDay                    (Command::COMMAND_YIELD_BY_DAY_QUERY,    Command::COMMAND_YIELD_BY_DAY_QUERY,    0, SpeedwireDataType::Yield, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "EYieldByDay");
const SpeedwireData SpeedwireData::Event                         (Command::COMMAND_EVENT_QUERY,           Command::COMMAND_EVENT_QUERY,           0, SpeedwireDataType::Event, 0, NULL, 0, MeasurementType::InverterStatus(), Wire::NO_WIRE, "Event");


/*******************************
 *  Class holding a map of SpeedwireData elements.
 ********************************/

SpeedwireDataMap SpeedwireDataMap::globalMap = SpeedwireDataMap(SpeedwireData::getAllPredefined());

/**
 *  Get a reference to the SpeedwireDataMap containing all predefined elements
 *  @return the map
 */
SpeedwireDataMap& SpeedwireDataMap::getGlobalMap(void) {
    if (globalMap.size() == 0) {
        globalMap = SpeedwireDataMap(SpeedwireData::getAllPredefined());
    }
    return globalMap;
}

