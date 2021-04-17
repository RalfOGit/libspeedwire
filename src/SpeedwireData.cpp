#include <memory.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireData.hpp>
#include <SpeedwireCommand.hpp>


/*******************************
 *  Class holding raw data from the speedwire inverter reply packet
 ********************************/
/**
 *  Constructor
 */
SpeedwireRawData::SpeedwireRawData(const uint32_t _command, const uint32_t _id, const uint8_t _conn, const uint8_t _type, const time_t _time, const void *const _data, const size_t _data_size) :
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
 *  Compare two instances of SpeedwireData with each other
 */
bool SpeedwireRawData::equals(const SpeedwireRawData& other) const {
    return (command == other.command && id == other.id && conn == other.conn && type == other.type && time == other.time && data_size == other.data_size && memcmp(data, other.data, data_size) == 0);
}


/**
 *  Compare two instance signatures of SpeedwireData with each other
 */
bool SpeedwireRawData::isSameSignature(const SpeedwireRawData& other) const {
    return (command == other.command && id == other.id && conn == other.conn && type == other.type);
}


/**
 *  Convert SpeedwireData into a std::string represenation
 */
std::string SpeedwireRawData::toString(void) const {
    char buff[256];
    snprintf(buff, sizeof(buff), "id 0x%08lx conn 0x%02x type 0x%02x  time 0x%08lx data 0x", (unsigned)id, (unsigned)conn, (unsigned)type, (uint32_t)time);
    std::string result(buff);
    for (size_t i = 0; i < data_size; ++i) {
        char byte[4];
        snprintf(byte, sizeof(byte), "%02x", (unsigned)data[i]);
        result.append(byte);
    }
    return result;
}

void SpeedwireRawData::print(uint32_t value, FILE* file) const {
    fprintf(file, "%s 0x%08lx %lu\n", toString().c_str(), value, value);
}

void SpeedwireRawData::print(uint64_t value, FILE* file) const {
    fprintf(file, "%s 0x%016llx %llu\n", toString().c_str(), value, value);
}



/*******************************
 *  Class holding data from the speedwire inverter reply packet, enriched by measurement type information
 *  and the interpreted measurement value
 ********************************/

/**
 *  Constructor
 */
SpeedwireData::SpeedwireData(const uint32_t command, const uint32_t id, const uint8_t conn, const uint8_t type, const time_t time, const void *const data, const size_t data_size,
                             const MeasurementType& mType, const Wire _wire) :
    SpeedwireRawData(command, id, conn, type, time, data, data_size),
    measurementType(mType),
    wire(_wire),
    description(mType.getFullName(_wire)),
    measurementValue() {
}


/**
 *  Default constructor, not very useful, but needed for std::map
 */
SpeedwireData::SpeedwireData(void) :
    SpeedwireRawData(0, 0, 0, 0, 0, NULL, 0),
    measurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::NO_QUANTITY, "", 0),
    wire(Wire::NO_WIRE),
    description(),
    measurementValue() {
}


/**
 *  Consume the given inverter data, i.e. interprete register id and convert to physical values
 */
bool SpeedwireData::consume(const SpeedwireRawData& data) {
    if (!isSameSignature(data)) return false;
    if (data.data == NULL || data.data_size < 20) return false;

    uint8_t  value1;
    uint32_t value4;

    switch (id) {
    case 0x00251e00:    // dc power
    case 0x00451f00:    // dc voltage
    case 0x00452100:    // dc current
    case 0x00464000:    // ac power
    case 0x00464100:    // ac power
    case 0x00464200:    // ac power
    case 0x00464800:    // ac voltage
    case 0x00464900:    // ac voltage
    case 0x00464a00:    // ac voltage
    case 0x00464b00:    // ac voltage
    case 0x00464c00:    // ac voltage
    case 0x00464d00:    // ac voltage
    case 0x00465300:    // ac current
    case 0x00465400:    // ac current
    case 0x00465500:    // ac current
        value4 = SpeedwireByteEncoding::getUint32LittleEndian(data.data);
        if (value4 == 0xffffffff || value4 == 0x80000000) value4 = 0;  // received during darkness
#if 0   // simulate some values
        if (id == 0x00251e00) value4 = 0x57;
        if (id == 0x00451f00) value4 = 0x6105;
        if (id == 0x00452100) value4 = 0x0160;
        if (id == 0x00464000 || id == 0x00464100 || id == 0x00464200) value4 = 0x0038;
        if (id == 0x00464800 || id == 0x00464900 || id == 0x00464a00) value4 = 0x59cf;
        if (id == 0x00464b00 || id == 0x00464c00 || id == 0x00464d00) value4 = 0x9b3c;
        if (id == 0x00465300 || id == 0x00465400 || id == 0x00465500) value4 = 0x011e;
#endif
        measurementValue.setValue(value4, measurementType.divisor);
        measurementValue.setTimer((uint32_t)data.time);
        time = data.time;
        break;

    case 0x00214800:    // device status
    case 0x00416400:    // grid relay status
        // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000980 00028051 00482100 ff482100 00000000 =>  query device status
        // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000980 01028051 00000000 00000000 01482108 59c5e95f 33010001 feffff00 00000000 00000000 00000000 00000000 00000000 00000000 00000000
        // Request  534d4100000402a00000000100260010 606509a0 7a01842a71b30001 7d0042be283a0001 000000000a80 00028051 00644100 ff644100 00000000 =>  query grid relay status
        // Response 534d4100000402a000000001004e0010 606513a0 7d0042be283a00a1 7a01842a71b30001 000000000a80 01028051 07000000 07000000 01644108 59c5e95f 33000001 37010000 fdffff00 feffff00 00000000 00000000 00000000 00000000 00000000
        value4 = SpeedwireByteEncoding::getUint32LittleEndian(data.data);
        value1 = (value4 >> 24) & 0xff;
        measurementValue.setValue((uint32_t)value1, measurementType.divisor);
        measurementValue.setTimer((uint32_t)data.time);
        time = data.time;
        break;

    default:
        perror("unknown id");
        return false;
    }

    return true;
}


/**
 *  Print instance to file
 */
void SpeedwireData::print(FILE* file) const {
    uint32_t timer = measurementValue.timer;
    double   value = measurementValue.value;
    fprintf(file, "%-16s  time %lu  %s  => %lf %s\n", description.c_str(), timer, SpeedwireRawData::toString().c_str(), value, measurementType.unit.c_str());
}


///**
// *  Add this instance to the given map, where the key is the command plus conn
// */
//void SpeedwireFilterData::addToMap(QueryMap& map) const {
//    map[id + conn] = *this;
//}
//
//
///**
// *  Search in the given map for an entry like the given template and add the entires measurement value to this instance
// */
//void SpeedwireFilterData::addValueFromMap(QueryMap& map, const SpeedwireFilterData &_template) {
//    auto iterator = map.find(_template.id | _template.conn);
//    if (iterator != map.end()) {
//        measurementValue->value += iterator->second.measurementValue->value;
//        measurementValue->timer  = iterator->second.measurementValue->timer;
//        time = iterator->second.time;
//    }
//}


// pre-defined instances
const SpeedwireData SpeedwireData::InverterPowerMPP1    (Command::COMMAND_DC_QUERY,     0x00251E00, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::MPP1);
const SpeedwireData SpeedwireData::InverterPowerMPP2    (Command::COMMAND_DC_QUERY,     0x00251E00, 0x02, 0x40, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::MPP2);
const SpeedwireData SpeedwireData::InverterVoltageMPP1  (Command::COMMAND_DC_QUERY,     0x00451F00, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::MPP1);
const SpeedwireData SpeedwireData::InverterVoltageMPP2  (Command::COMMAND_DC_QUERY,     0x00451F00, 0x02, 0x40, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::MPP2);
const SpeedwireData SpeedwireData::InverterCurrentMPP1  (Command::COMMAND_DC_QUERY,     0x00452100, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::MPP1);
const SpeedwireData SpeedwireData::InverterCurrentMPP2  (Command::COMMAND_DC_QUERY,     0x00452100, 0x02, 0x40, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::MPP2);
const SpeedwireData SpeedwireData::InverterPowerL1      (Command::COMMAND_AC_QUERY,     0x00464000, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L1);
const SpeedwireData SpeedwireData::InverterPowerL2      (Command::COMMAND_AC_QUERY,     0x00464100, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L2);
const SpeedwireData SpeedwireData::InverterPowerL3      (Command::COMMAND_AC_QUERY,     0x00464200, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterPower(),   Wire::L3);
const SpeedwireData SpeedwireData::InverterVoltageL1    (Command::COMMAND_AC_QUERY,     0x00464800, 0x01, 0x00, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1);    // L1 -> N
const SpeedwireData SpeedwireData::InverterVoltageL2    (Command::COMMAND_AC_QUERY,     0x00464900, 0x01, 0x00, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2);    // L2 -> N
const SpeedwireData SpeedwireData::InverterVoltageL3    (Command::COMMAND_AC_QUERY,     0x00464a00, 0x01, 0x00, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3);    // L3 -> N
const SpeedwireData SpeedwireData::InverterVoltageL1toL2(Command::COMMAND_AC_QUERY,     0x00464b00, 0x01, 0x00, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L1);    // L1 -> L2
const SpeedwireData SpeedwireData::InverterVoltageL2toL3(Command::COMMAND_AC_QUERY,     0x00464c00, 0x01, 0x00, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L2);    // L2 -> L3
const SpeedwireData SpeedwireData::InverterVoltageL3toL1(Command::COMMAND_AC_QUERY,     0x00464d00, 0x01, 0x00, 0, NULL, 0, MeasurementType::InverterVoltage(), Wire::L3);    // L3 -> L1
const SpeedwireData SpeedwireData::InverterCurrentL1    (Command::COMMAND_AC_QUERY,     0x00465300, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L1);
const SpeedwireData SpeedwireData::InverterCurrentL2    (Command::COMMAND_AC_QUERY,     0x00465400, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L2);
const SpeedwireData SpeedwireData::InverterCurrentL3    (Command::COMMAND_AC_QUERY,     0x00465500, 0x01, 0x40, 0, NULL, 0, MeasurementType::InverterCurrent(), Wire::L3);
const SpeedwireData SpeedwireData::InverterStatus       (Command::COMMAND_STATUS_QUERY, 0x00214800, 0x01, 0x08, 0, NULL, 0, MeasurementType::InverterStatus(),  Wire::DEVICE_OK);
const SpeedwireData SpeedwireData::InverterRelay        (Command::COMMAND_STATUS_QUERY, 0x00416400, 0x01, 0x08, 0, NULL, 0, MeasurementType::InverterRelay(),   Wire::RELAY_ON);

// pre-defined instances of derived measurement values
const SpeedwireData SpeedwireData::InverterPowerDCTotal   (0, 0, 0, 0, 0, NULL, 0, MeasurementType::InverterPower(),      Wire::MPP_TOTAL);
const SpeedwireData SpeedwireData::InverterPowerACTotal   (0, 0, 0, 0, 0, NULL, 0, MeasurementType::InverterPower(),      Wire::TOTAL);
const SpeedwireData SpeedwireData::InverterPowerLoss      (0, 0, 0, 0, 0, NULL, 0, MeasurementType::InverterLoss(),       Wire::LOSS_TOTAL);
const SpeedwireData SpeedwireData::InverterPowerEfficiency(0, 0, 0, 0, 0, NULL, 0, MeasurementType::InverterEfficiency(), Wire::NO_WIRE);
