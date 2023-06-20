#include <memory.h>
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
    const SpeedwireDataMap &data_map = SpeedwireDataMap::getAllPredefined();
    auto iterator = data_map.find(toKey());
    if (iterator != data_map.end()) {
        description = iterator->second.description;
        //unsigned long divisor = iterator->second.measurementType.divisor;
    }

    // assemble a string from the header fields
    char buff[256];
    snprintf(buff, sizeof(buff), "id 0x%08lx (%12s) conn 0x%02x type 0x%02x (%10s)  time 0x%08lx  data ", (unsigned)id, description.c_str(), (unsigned)conn, (unsigned)type, libspeedwire::toString(type).c_str(),  (uint32_t)time);

    // decode values and append them to the string
    std::string result(buff);
    size_t num_values = getNumberOfValues();
    for (size_t i = 0; i < num_values; ++i) {
        char byte[32];
        switch (type) {
        case SpeedwireDataType::Signed32: {
            int32_t value = getValueAsSignedLong(i);
            if (value == 0x80000000) { result.append("        NaN"); break; }
            snprintf(byte, sizeof(byte), " %10ld", value);
            result.append(byte);
            break;
        }
        case SpeedwireDataType::Unsigned32: {
            uint32_t value = getValueAsUnsignedLong(i);
            if (value == 0xffffffff) { result.append("        NaN"); break; }
            snprintf(byte, sizeof(byte), " %10lu", value);
            result.append(byte);
            break;
        }
        case SpeedwireDataType::Status32: {
            uint32_t value = getValueAsUnsignedLong(i);
            if (value == 0x00fffffd) { result.append("        NaN"); break; }
            if (value == 0x00fffffe) { result.append("        EoD"); break; }
            snprintf(byte, sizeof(byte), " 0x%08lx", value);
            result.append(byte);
            break;
        }
        case SpeedwireDataType::Float:
            snprintf(byte, sizeof(byte), " %7.2f", getValueAsFloat(i));
            result.append(byte);
            break;
        case SpeedwireDataType::String32:
            result.append(" ");
            result.append(getValueAsString(0));
            break;
        }
    }
    return result;
}


/** 
 *  Get number of data values available in the payload data 
 */
size_t SpeedwireRawData::getNumberOfValues(void) const {
    switch (type) {
    case SpeedwireDataType::Unsigned32:
    case SpeedwireDataType::Status32:
    case SpeedwireDataType::Float:
    case SpeedwireDataType::Signed32:
        return data_size / 4u;
        return 2;
    case SpeedwireDataType::String32:
        return 1;
    }
    return 0;
}


/** Get data value from payload at the given position */
uint32_t SpeedwireRawData::getValueAsUnsignedLong(size_t pos) const {
    if (pos < getNumberOfValues()) {
        uint32_t value = SpeedwireByteEncoding::getUint32LittleEndian(data + pos * 4u);
        return value;
    }
    return 0x00fffffe;
}


int32_t SpeedwireRawData::getValueAsSignedLong(size_t pos) const {
    return (int32_t)getValueAsUnsignedLong(pos);
}


float SpeedwireRawData::getValueAsFloat(size_t pos) const {
    uint32_t uint_value = getValueAsUnsignedLong(pos);
    float value = 0.0f;
    memcpy(&value, &uint_value, sizeof(value));
    return value;
}


std::string SpeedwireRawData::getValueAsString(size_t pos) const {
    std::string value;
    if (pos < data_size) {
        value.append((char*)data + pos, data_size - pos);
    }
    return value;
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
                             const MeasurementType& mType, const Wire _wire) :
    SpeedwireRawData(command, id, conn, type, time, data, data_size),
    Measurement(mType, _wire) {
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
        int32_t value = (int32_t)SpeedwireByteEncoding::getUint32LittleEndian(data.data);
        if (value == 0x80000000) value = 0;  // received during darkness: NaN value is 0x80000000
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
        uint32_t value = SpeedwireByteEncoding::getUint32LittleEndian(data.data);
        if (value == 0xffffffff) value = 0;  // received during darkness: NaN value is 0xffffffff
        addMeasurement(value, (uint32_t)data.time);
        time = data.time;
        break;
    }

    case SpeedwireDataType::Status32: {
        switch (id) {
        case 0x00214800:    // device status
        case 0x00416400: {  // grid relay status
            // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000980 00028051 00482100 ff482100 00000000 =>  query device status
            // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000980 01028051 00000000 00000000 01482108 59c5e95f 33010001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000 00000000
            // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000a80 00028051 00644100 ff644100 00000000 =>  query grid relay status
            // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000a80 01028051 07000000 07000000 01644108 59c5e95f 33000001 37010000 fdffff00 feffff00 00000000 00000000 00000000 00000000 00000000
            uint32_t value32 = SpeedwireByteEncoding::getUint32LittleEndian(data.data);
            //if (value32 == 0x00fffffd) value32 = 0;  // NaN value is 0x00fffffd
            uint8_t  valued8 = (value32 >> 24) & 0xff;
            addMeasurement((uint32_t)valued8, (uint32_t)data.time);
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

    predefined.push_back(InverterPowerMPP1);
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
    predefined.push_back(InverterCurrentL1);
    predefined.push_back(InverterCurrentL2);
    predefined.push_back(InverterCurrentL3);
    predefined.push_back(InverterStatus);
    predefined.push_back(InverterRelay);

    predefined.push_back(InverterPowerDCTotal);
    predefined.push_back(InverterPowerACTotal);
    predefined.push_back(InverterPowerLoss);
    predefined.push_back(InverterPowerEfficiency);

    predefined.push_back(HouseholdPowerTotal);
    predefined.push_back(HouseholdIncomeTotal);
    predefined.push_back(HouseholdIncomeFeedIn);
    predefined.push_back(HouseholdIncomeSelfConsumption);

    return predefined;
}


// pre-defined SpeedwireData instances
const SpeedwireData SpeedwireData::InverterPowerMPP1    (Command::COMMAND_DC_QUERY,     0x00251E00, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::MPP1);
const SpeedwireData SpeedwireData::InverterPowerMPP2    (Command::COMMAND_DC_QUERY,     0x00251E00, 0x02, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::MPP2);
const SpeedwireData SpeedwireData::InverterVoltageMPP1  (Command::COMMAND_DC_QUERY,     0x00451F00, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::MPP1);
const SpeedwireData SpeedwireData::InverterVoltageMPP2  (Command::COMMAND_DC_QUERY,     0x00451F00, 0x02, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::MPP2);
const SpeedwireData SpeedwireData::InverterCurrentMPP1  (Command::COMMAND_DC_QUERY,     0x00452100, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::MPP1);
const SpeedwireData SpeedwireData::InverterCurrentMPP2  (Command::COMMAND_DC_QUERY,     0x00452100, 0x02, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::MPP2);
const SpeedwireData SpeedwireData::InverterPowerL1      (Command::COMMAND_AC_QUERY,     0x00464000, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L1);
const SpeedwireData SpeedwireData::InverterPowerL2      (Command::COMMAND_AC_QUERY,     0x00464100, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L2);
const SpeedwireData SpeedwireData::InverterPowerL3      (Command::COMMAND_AC_QUERY,     0x00464200, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L3);
const SpeedwireData SpeedwireData::InverterVoltageL1    (Command::COMMAND_AC_QUERY,     0x00464800, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1);    // L1 -> N
const SpeedwireData SpeedwireData::InverterVoltageL2    (Command::COMMAND_AC_QUERY,     0x00464900, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2);    // L2 -> N
const SpeedwireData SpeedwireData::InverterVoltageL3    (Command::COMMAND_AC_QUERY,     0x00464a00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3);    // L3 -> N
const SpeedwireData SpeedwireData::InverterVoltageL1toL2(Command::COMMAND_AC_QUERY,     0x00464b00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1);    // L1 -> L2
const SpeedwireData SpeedwireData::InverterVoltageL2toL3(Command::COMMAND_AC_QUERY,     0x00464c00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2);    // L2 -> L3
const SpeedwireData SpeedwireData::InverterVoltageL3toL1(Command::COMMAND_AC_QUERY,     0x00464d00, 0x01, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3);    // L3 -> L1
const SpeedwireData SpeedwireData::InverterCurrentL1    (Command::COMMAND_AC_QUERY,     0x00465300, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L1);
const SpeedwireData SpeedwireData::InverterCurrentL2    (Command::COMMAND_AC_QUERY,     0x00465400, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L2);
const SpeedwireData SpeedwireData::InverterCurrentL3    (Command::COMMAND_AC_QUERY,     0x00465500, 0x01, SpeedwireDataType::Signed32, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L3);
const SpeedwireData SpeedwireData::InverterStatus       (Command::COMMAND_STATUS_QUERY, 0x00214800, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterStatus(),  Wire::DEVICE_OK);
const SpeedwireData SpeedwireData::InverterRelay        (Command::COMMAND_STATUS_QUERY, 0x00416400, 0x01, SpeedwireDataType::Status32, 0, NULL, 0, MeasurementType::InverterRelay(),   Wire::RELAY_ON);

// pre-defined instances of derived measurement values
const SpeedwireData SpeedwireData::InverterPowerDCTotal   (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(),      Wire::MPP_TOTAL);
const SpeedwireData SpeedwireData::InverterPowerACTotal   (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(),      Wire::TOTAL);
const SpeedwireData SpeedwireData::InverterPowerLoss      (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterLoss(),       Wire::LOSS_TOTAL);
const SpeedwireData SpeedwireData::InverterPowerEfficiency(0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterEfficiency(), Wire::NO_WIRE);

// pre-defined instances for miscellaneous measurement types
const SpeedwireData SpeedwireData::HouseholdPowerTotal           (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::InverterPower(), Wire::TOTAL);
const SpeedwireData SpeedwireData::HouseholdIncomeTotal          (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::Currency(),      Wire::TOTAL);
const SpeedwireData SpeedwireData::HouseholdIncomeFeedIn         (0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::Currency(),      Wire::FEED_IN);
const SpeedwireData SpeedwireData::HouseholdIncomeSelfConsumption(0, 0, 0, SpeedwireDataType::Unsigned32, 0, NULL, 0, MeasurementType::Currency(),      Wire::SELF_CONSUMPTION);


/*******************************
 *  Class holding a map of SpeedwireData elements.
 ********************************/

/**
 *  Create a SpeedwireDataMap from the given vector of SpeedwireData elements
 *  @param elements the vector of SpeedwireData elements
 *  @return the map
 */
SpeedwireDataMap SpeedwireDataMap::createMap(const std::vector<SpeedwireData>& elements) {
    SpeedwireDataMap map;
    for (auto& e : elements) {
        map.add(e);
    }
    return map;
}


/**
 *  Get a reference to the SpeedwireDataMap containing all predefined elements
 *  @return the map
 */
const SpeedwireDataMap& SpeedwireDataMap::getAllPredefined(void) {
    if (allPredefined.size() == 0) {
        allPredefined = createMap(SpeedwireData::getAllPredefined());
    }
    return allPredefined;
}

SpeedwireDataMap SpeedwireDataMap::allPredefined;
