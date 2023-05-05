#include <ObisData.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#pragma warning( disable : 4996 )  // disable sscanf_s suggestion
using namespace libspeedwire;


/**
 *  Constructor.
 *  @param channel obis measurement channel
 *  @param index   obis measurement index; i.e. measurement quantity
 *  @param type    obis measurement type
 *  @param tariff  obis tariff
 */
ObisType::ObisType(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff) {
    this->channel = channel;
    this->index = index;
    this->type = type;
    this->tariff = tariff;
}

//! Equals operator - compares this obis instance with another ObisType instance
bool ObisType::equals(const ObisType &other) const {
    return (channel == other.channel && index == other.index && type == other.type && tariff == other.tariff);
}

//! Convert this instance to a string
std::string ObisType::toString(void) const {
    char str[16];
    snprintf(str, sizeof(str), "%d.%02d.%d.%d", channel, index, type, tariff);
    return std::string(str);
}

//! Convert this instance augmented by the given uint32 value to a string
std::string ObisType::toString(const uint32_t value) const {
    char str[64];
    snprintf(str, sizeof(str), "%s 0x%08lx %lu", toString().c_str(), value, value);
    return std::string(str);
}

//! Convert this instance augmented by the given uint64 value to a string
std::string ObisType::toString(const uint64_t value) const {
    char str[64];
    snprintf(str, sizeof(str), "%s 0x%016llx %llu", toString().c_str(), value, value);
    return std::string(str);
}

//! Convert this instance to its byte encoding; additional 8 bytes are available to encode a 4 byte or 8 byte obis value
std::array<uint8_t, 12> ObisType::toByteArray(void) const {
    std::array<uint8_t, 12> bytes = { channel, index, type, tariff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    return bytes;
}

//! Convert this instance into a key that can be used by std::map<uint32_t, ...>.
uint32_t ObisType::toKey(void) const {
    uint32_t key = ((uint32_t)channel << 24) | ((uint32_t)index << 16) | ((uint32_t)type << 8) | (uint32_t)tariff;
    return key;
}


// =================================================================================================

/**
 *  Default constructor - not very useful but required for std::map.
 */
ObisData::ObisData(void) :
    ObisType(0, 0, 0, 0),
    Measurement(MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::NO_QUANTITY, "", 0), Wire::NO_WIRE) {
    description = "";
}

/**
 *  Constructor.
 *  @param channel obis measurement channel
 *  @param index   obis measurement index; i.e. measurement quantity
 *  @param type    obis measurement type
 *  @param tariff  obis tariff
 *  @param mType   measurement type
 *  @param wire_   measurement wire
 */
ObisData::ObisData(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff,
                   const MeasurementType &mType, const Wire &wire_) : 
    ObisType(channel, index, type, tariff),
    Measurement(mType, wire_) {
}

//! Equals operator - compares this instance with the given ObisType instance (i.e. just using information from base class ObisType)
bool ObisData::equals(const ObisType &other) const {
    return ObisType::equals(other);
}

//! Print this instance to file
void ObisData::print(FILE *file) const {
    TimestampDoublePair measurementValue = measurementValues.getNewestElement();
    uint32_t    timer  = measurementValue.time;
    double      value  = measurementValue.value;
    std::string string = measurementValues.value_string;
    if (string.length() > 0) {
        fprintf(file, "%-31s  %lu  %s  => %s\n", description.c_str(), timer, ObisType::toString().c_str(), string.c_str());
    }
    else {
        fprintf(file, "%-31s  %lu  %s  => %lf %s\n", description.c_str(), timer, ObisType::toString().c_str(), value, measurementType.unit.c_str());
    }
}

//! Convert this instance into its byte array representation according to the obis byte stream definition
std::array<uint8_t, 12> ObisData::toByteArray(void) const {
    TimestampDoublePair measurementValue = measurementValues.getNewestElement();
    std::array<uint8_t, 12> byte_array = ObisType::toByteArray();
    switch (type) {
    case 0:
        if (channel == 144) {
            // convert software version
            uint32_t int_array[sizeof(uint32_t)] = { 0xff, 0xff, 0xff, 0xff };
            int n = sscanf(measurementValues.value_string.c_str(), "%u.%u.%u.%u", &int_array[3], &int_array[2], &int_array[1], &int_array[0]);
            if (n != 4) {
                n = sscanf(measurementValues.value_string.c_str(), "%02x.%02x.%02x.%02x", &int_array[3], &int_array[2], &int_array[1], &int_array[0]);
            }
            uint32_t value = int_array[3] << 24 | int_array[2] << 16 | int_array[1] << 8 | int_array[0];
            SpeedwireEmeterProtocol::setObisValue4(byte_array.data(), value);
        }
        else if (channel == 0 && index == 0 && tariff == 0) {
            SpeedwireEmeterProtocol::setObisValue4(byte_array.data(), 0);
        }
        break;
    case 4:
    case 7:
        SpeedwireEmeterProtocol::setObisValue4(byte_array.data(), (uint32_t)(measurementValue.value * measurementType.divisor));
        break;
    case 8:
        SpeedwireEmeterProtocol::setObisValue8(byte_array.data(), (uint64_t)(measurementValue.value * measurementType.divisor));
        break;
    }
    return byte_array;
}

//! Get a vector of all pre-defined ObisData instances - they are defined in the order they appear in an emeter packet
std::vector<ObisData> ObisData::getAllPredefined(void) {
    std::vector<ObisData> predefined;

    // totals
    predefined.push_back(PositiveActivePowerTotal);
    predefined.push_back(PositiveActiveEnergyTotal);
    predefined.push_back(NegativeActivePowerTotal);
    predefined.push_back(NegativeActiveEnergyTotal);
    predefined.push_back(PositiveReactivePowerTotal);
    predefined.push_back(PositiveReactiveEnergyTotal);
    predefined.push_back(NegativeReactivePowerTotal);
    predefined.push_back(NegativeReactiveEnergyTotal);
    predefined.push_back(PositiveApparentPowerTotal);
    predefined.push_back(PositiveApparentEnergyTotal);
    predefined.push_back(NegativeApparentPowerTotal);
    predefined.push_back(NegativeApparentEnergyTotal);
    predefined.push_back(PowerFactorTotal);
    predefined.push_back(Frequency);

    // line 1
    predefined.push_back(PositiveActivePowerL1);
    predefined.push_back(PositiveActiveEnergyL1);
    predefined.push_back(NegativeActivePowerL1);
    predefined.push_back(NegativeActiveEnergyL1);
    predefined.push_back(PositiveReactivePowerL1);
    predefined.push_back(PositiveReactiveEnergyL1);
    predefined.push_back(NegativeReactivePowerL1);
    predefined.push_back(NegativeReactiveEnergyL1);
    predefined.push_back(PositiveApparentPowerL1);
    predefined.push_back(PositiveApparentEnergyL1);
    predefined.push_back(NegativeApparentPowerL1);
    predefined.push_back(NegativeApparentEnergyL1);
    predefined.push_back(CurrentL1);
    predefined.push_back(VoltageL1);
    predefined.push_back(PowerFactorL1);

    // line 2
    predefined.push_back(PositiveActivePowerL2);
    predefined.push_back(PositiveActiveEnergyL2);
    predefined.push_back(NegativeActivePowerL2);
    predefined.push_back(NegativeActiveEnergyL2);
    predefined.push_back(PositiveReactivePowerL2);
    predefined.push_back(PositiveReactiveEnergyL2);
    predefined.push_back(NegativeReactivePowerL2);
    predefined.push_back(NegativeReactiveEnergyL2);
    predefined.push_back(PositiveApparentPowerL2);
    predefined.push_back(PositiveApparentEnergyL2);
    predefined.push_back(NegativeApparentPowerL2);
    predefined.push_back(NegativeApparentEnergyL2);
    predefined.push_back(CurrentL2);
    predefined.push_back(VoltageL2);
    predefined.push_back(PowerFactorL2);

    // line 3
    predefined.push_back(PositiveActivePowerL3);
    predefined.push_back(PositiveActiveEnergyL3);
    predefined.push_back(NegativeActivePowerL3);
    predefined.push_back(NegativeActiveEnergyL3);
    predefined.push_back(PositiveReactivePowerL3);
    predefined.push_back(PositiveReactiveEnergyL3);
    predefined.push_back(NegativeReactivePowerL3);
    predefined.push_back(NegativeReactiveEnergyL3);
    predefined.push_back(PositiveApparentPowerL3);
    predefined.push_back(PositiveApparentEnergyL3);
    predefined.push_back(NegativeApparentPowerL3);
    predefined.push_back(NegativeApparentEnergyL3);
    predefined.push_back(CurrentL3);
    predefined.push_back(VoltageL3);
    predefined.push_back(PowerFactorL3);

    // software version
    predefined.push_back(SoftwareVersion);
    predefined.push_back(EndOfData);

    // calculated value, not part of an emeter packet
    predefined.push_back(SignedActivePowerTotal);
    predefined.push_back(SignedActivePowerL1);
    predefined.push_back(SignedActivePowerL2);
    predefined.push_back(SignedActivePowerL3);
    return predefined;
}


// definition of pre-defined instances
const ObisData ObisData::PositiveActivePowerTotal   (0,  1, 4, 0, MeasurementType::EmeterPositiveActivePower(),    Wire::TOTAL);
const ObisData ObisData::PositiveActivePowerL1      (0, 21, 4, 0, MeasurementType::EmeterPositiveActivePower(),    Wire::L1);
const ObisData ObisData::PositiveActivePowerL2      (0, 41, 4, 0, MeasurementType::EmeterPositiveActivePower(),    Wire::L2);
const ObisData ObisData::PositiveActivePowerL3      (0, 61, 4, 0, MeasurementType::EmeterPositiveActivePower(),    Wire::L3);
const ObisData ObisData::PositiveActiveEnergyTotal  (0,  1, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(),   Wire::TOTAL);
const ObisData ObisData::PositiveActiveEnergyL1     (0, 21, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(),   Wire::L1);
const ObisData ObisData::PositiveActiveEnergyL2     (0, 41, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(),   Wire::L2);
const ObisData ObisData::PositiveActiveEnergyL3     (0, 61, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(),   Wire::L3);
const ObisData ObisData::NegativeActivePowerTotal   (0,  2, 4, 0, MeasurementType::EmeterNegativeActivePower(),    Wire::TOTAL);
const ObisData ObisData::NegativeActivePowerL1      (0, 22, 4, 0, MeasurementType::EmeterNegativeActivePower(),    Wire::L1);
const ObisData ObisData::NegativeActivePowerL2      (0, 42, 4, 0, MeasurementType::EmeterNegativeActivePower(),    Wire::L2);
const ObisData ObisData::NegativeActivePowerL3      (0, 62, 4, 0, MeasurementType::EmeterNegativeActivePower(),    Wire::L3);
const ObisData ObisData::NegativeActiveEnergyTotal  (0,  2, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(),   Wire::TOTAL);
const ObisData ObisData::NegativeActiveEnergyL1     (0, 22, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(),   Wire::L1);
const ObisData ObisData::NegativeActiveEnergyL2     (0, 42, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(),   Wire::L2);
const ObisData ObisData::NegativeActiveEnergyL3     (0, 62, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(),   Wire::L3);
const ObisData ObisData::PositiveReactivePowerTotal (0,  3, 4, 0, MeasurementType::EmeterPositiveReactivePower(),  Wire::TOTAL);
const ObisData ObisData::PositiveReactivePowerL1    (0, 23, 4, 0, MeasurementType::EmeterPositiveReactivePower(),  Wire::L1);
const ObisData ObisData::PositiveReactivePowerL2    (0, 43, 4, 0, MeasurementType::EmeterPositiveReactivePower(),  Wire::L2);
const ObisData ObisData::PositiveReactivePowerL3    (0, 63, 4, 0, MeasurementType::EmeterPositiveReactivePower(),  Wire::L3);
const ObisData ObisData::PositiveReactiveEnergyTotal(0,  3, 8, 0, MeasurementType::EmeterPositiveReactiveEnergy(), Wire::TOTAL);
const ObisData ObisData::PositiveReactiveEnergyL1   (0, 23, 8, 0, MeasurementType::EmeterPositiveReactiveEnergy(), Wire::L1);
const ObisData ObisData::PositiveReactiveEnergyL2   (0, 43, 8, 0, MeasurementType::EmeterPositiveReactiveEnergy(), Wire::L2);
const ObisData ObisData::PositiveReactiveEnergyL3   (0, 63, 8, 0, MeasurementType::EmeterPositiveReactiveEnergy(), Wire::L3);
const ObisData ObisData::NegativeReactivePowerTotal (0,  4, 4, 0, MeasurementType::EmeterNegativeReactivePower(),  Wire::TOTAL);
const ObisData ObisData::NegativeReactivePowerL1    (0, 24, 4, 0, MeasurementType::EmeterNegativeReactivePower(),  Wire::L1);
const ObisData ObisData::NegativeReactivePowerL2    (0, 44, 4, 0, MeasurementType::EmeterNegativeReactivePower(),  Wire::L2);
const ObisData ObisData::NegativeReactivePowerL3    (0, 64, 4, 0, MeasurementType::EmeterNegativeReactivePower(),  Wire::L3);
const ObisData ObisData::NegativeReactiveEnergyTotal(0,  4, 8, 0, MeasurementType::EmeterNegativeReactiveEnergy(), Wire::TOTAL);
const ObisData ObisData::NegativeReactiveEnergyL1   (0, 24, 8, 0, MeasurementType::EmeterNegativeReactiveEnergy(), Wire::L1);
const ObisData ObisData::NegativeReactiveEnergyL2   (0, 44, 8, 0, MeasurementType::EmeterNegativeReactiveEnergy(), Wire::L2);
const ObisData ObisData::NegativeReactiveEnergyL3   (0, 64, 8, 0, MeasurementType::EmeterNegativeReactiveEnergy(), Wire::L3);
const ObisData ObisData::PositiveApparentPowerTotal (0,  9, 4, 0, MeasurementType::EmeterPositiveApparentPower(),  Wire::TOTAL);
const ObisData ObisData::PositiveApparentPowerL1    (0, 29, 4, 0, MeasurementType::EmeterPositiveApparentPower(),  Wire::L1);
const ObisData ObisData::PositiveApparentPowerL2    (0, 49, 4, 0, MeasurementType::EmeterPositiveApparentPower(),  Wire::L2);
const ObisData ObisData::PositiveApparentPowerL3    (0, 69, 4, 0, MeasurementType::EmeterPositiveApparentPower(),  Wire::L3);
const ObisData ObisData::PositiveApparentEnergyTotal(0,  9, 8, 0, MeasurementType::EmeterPositiveApparentEnergy(), Wire::TOTAL);
const ObisData ObisData::PositiveApparentEnergyL1   (0, 29, 8, 0, MeasurementType::EmeterPositiveApparentEnergy(), Wire::L1);
const ObisData ObisData::PositiveApparentEnergyL2   (0, 49, 8, 0, MeasurementType::EmeterPositiveApparentEnergy(), Wire::L2);
const ObisData ObisData::PositiveApparentEnergyL3   (0, 69, 8, 0, MeasurementType::EmeterPositiveApparentEnergy(), Wire::L3);
const ObisData ObisData::NegativeApparentPowerTotal (0, 10, 4, 0, MeasurementType::EmeterNegativeApparentPower(),  Wire::TOTAL);
const ObisData ObisData::NegativeApparentPowerL1    (0, 30, 4, 0, MeasurementType::EmeterNegativeApparentPower(),  Wire::L1);
const ObisData ObisData::NegativeApparentPowerL2    (0, 50, 4, 0, MeasurementType::EmeterNegativeApparentPower(),  Wire::L2);
const ObisData ObisData::NegativeApparentPowerL3    (0, 70, 4, 0, MeasurementType::EmeterNegativeApparentPower(),  Wire::L3);
const ObisData ObisData::NegativeApparentEnergyTotal(0, 10, 8, 0, MeasurementType::EmeterNegativeApparentEnergy(), Wire::TOTAL);
const ObisData ObisData::NegativeApparentEnergyL1   (0, 30, 8, 0, MeasurementType::EmeterNegativeApparentEnergy(), Wire::L1);
const ObisData ObisData::NegativeApparentEnergyL2   (0, 50, 8, 0, MeasurementType::EmeterNegativeApparentEnergy(), Wire::L2);
const ObisData ObisData::NegativeApparentEnergyL3   (0, 70, 8, 0, MeasurementType::EmeterNegativeApparentEnergy(), Wire::L3);
const ObisData ObisData::PowerFactorTotal           (0, 13, 4, 0, MeasurementType::EmeterPowerFactor(),            Wire::TOTAL);
const ObisData ObisData::Frequency                  (0, 14, 4, 0, MeasurementType::EmeterFrequency(),              Wire::TOTAL);
const ObisData ObisData::CurrentL1                  (0, 31, 4, 0, MeasurementType::EmeterCurrent(),                Wire::L1);
const ObisData ObisData::CurrentL2                  (0, 51, 4, 0, MeasurementType::EmeterCurrent(),                Wire::L2);
const ObisData ObisData::CurrentL3                  (0, 71, 4, 0, MeasurementType::EmeterCurrent(),                Wire::L3);
const ObisData ObisData::VoltageL1                  (0, 32, 4, 0, MeasurementType::EmeterVoltage(),                Wire::L1);
const ObisData ObisData::VoltageL2                  (0, 52, 4, 0, MeasurementType::EmeterVoltage(),                Wire::L2);
const ObisData ObisData::VoltageL3                  (0, 72, 4, 0, MeasurementType::EmeterVoltage(),                Wire::L3);
const ObisData ObisData::PowerFactorL1              (0, 33, 4, 0, MeasurementType::EmeterPowerFactor(),            Wire::L1);
const ObisData ObisData::PowerFactorL2              (0, 53, 4, 0, MeasurementType::EmeterPowerFactor(),            Wire::L2);
const ObisData ObisData::PowerFactorL3              (0, 73, 4, 0, MeasurementType::EmeterPowerFactor(),            Wire::L3);
const ObisData ObisData::SoftwareVersion            (144,0, 0, 0, MeasurementType::EmeterSoftwareVersion(),        Wire::NO_WIRE);
const ObisData ObisData::EndOfData                  (0,  0, 0, 0, MeasurementType::EmeterEndOfData(),              Wire::NO_WIRE);
const ObisData ObisData::SignedActivePowerTotal     (0, 16, 7, 0, MeasurementType::EmeterSignedActivePower(),      Wire::TOTAL); 
const ObisData ObisData::SignedActivePowerL1        (0, 36, 7, 0, MeasurementType::EmeterSignedActivePower(),      Wire::L1); 
const ObisData ObisData::SignedActivePowerL2        (0, 56, 7, 0, MeasurementType::EmeterSignedActivePower(),      Wire::L2);
const ObisData ObisData::SignedActivePowerL3        (0, 76, 7, 0, MeasurementType::EmeterSignedActivePower(),      Wire::L3);
