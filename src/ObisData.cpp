#include <ObisData.hpp>


/**
 *  Class holding an obis data definition used inside speedwire emeter packets
 */
ObisType::ObisType(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff) {
    this->channel = channel;
    this->index = index;
    this->type = type;
    this->tariff = tariff;
}

bool ObisType::equals(const ObisType &other) const {
    return (channel == other.channel && index == other.index && type == other.type && tariff == other.tariff);
}

std::string ObisType::toString(void) const {
    char str[16];
    snprintf(str, sizeof(str), "%d.%02d.%d.%d", channel, index, type, tariff);
    return std::string(str);
}

void ObisType::print(const uint32_t value, FILE *file) const {
    fprintf(file, "%s 0x%08lx %lu\n", toString().c_str(), value, value);
}

void ObisType::print(const uint64_t value, FILE *file) const {
    fprintf(file, "%s 0x%016llx %llu\n", toString().c_str(), value, value);
}

std::array<uint8_t, 8> ObisType::toByteArray(void) const {
    std::array<uint8_t, 8> bytes = { channel, index, type, tariff, 0, 0, 0, 0 };
    return bytes;
}


/**
 *  Class holding an emeter measurement together with its corresponding obis data definition and measurement type definition
 */
ObisData::ObisData(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff,
                               const MeasurementType &mType, const Line &lin) : 
    ObisType(channel, index, type, tariff),
    measurementType(mType),
    line(lin),
    description(mType.getFullName(lin)) {
    measurementValue = new MeasurementValue();
}

// copy constructor
ObisData::ObisData(const ObisData &rhs) :
    ObisData(rhs.channel, rhs.index, rhs.type, rhs.tariff, rhs.measurementType, Line::TOTAL) {
    line = rhs.line;
    description = rhs.description;
    *measurementValue = *rhs.measurementValue;  // the constructor call above already allocated a new MeasurementValue instance
}

// assignment operator
ObisData &ObisData::operator=(const ObisData &rhs) {
    if (this != &rhs) {
        this->ObisType::operator=(rhs);
        this->measurementType = rhs.measurementType;
        this->line = rhs.line;
        this->description = rhs.description;
        *this->measurementValue = *rhs.measurementValue;
    }
    return *this;
}

// destructor
ObisData::~ObisData(void) {
    if (measurementValue != NULL) {
        delete measurementValue;
        measurementValue = NULL;
    }
}

// equals operator - just using information from base class ObisType
bool ObisData::equals(const ObisType &other) const {
    return ObisType::equals(other);
}

// print instance to file
void ObisData::print(FILE *file) const {
    uint32_t timer = (measurementValue != NULL ? measurementValue->timer : 0xfffffffful);
    double   value = (measurementValue != NULL ? measurementValue->value : -999999.9999);
    fprintf(file, "%-25s  %lu  %s  => %lf %s\n", description.c_str(), timer, ObisType::toString().c_str(), value, measurementType.unit.c_str());
}

// definition of pre-defined instances
const ObisData ObisData::PositiveActivePowerTotal (0,  1, 4, 0, MeasurementType::EmeterPositiveActivePower(),  Line::TOTAL);
const ObisData ObisData::PositiveActivePowerL1    (0, 21, 4, 0, MeasurementType::EmeterPositiveActivePower(),  Line::L1);
const ObisData ObisData::PositiveActivePowerL2    (0, 41, 4, 0, MeasurementType::EmeterPositiveActivePower(),  Line::L2);
const ObisData ObisData::PositiveActivePowerL3    (0, 61, 4, 0, MeasurementType::EmeterPositiveActivePower(),  Line::L3);
const ObisData ObisData::PositiveActiveEnergyTotal(0,  1, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(), Line::TOTAL);
const ObisData ObisData::PositiveActiveEnergyL1   (0, 21, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(), Line::L1);
const ObisData ObisData::PositiveActiveEnergyL2   (0, 41, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(), Line::L2);
const ObisData ObisData::PositiveActiveEnergyL3   (0, 61, 8, 0, MeasurementType::EmeterPositiveActiveEnergy(), Line::L3);
const ObisData ObisData::NegativeActivePowerTotal (0,  2, 4, 0, MeasurementType::EmeterNegativeActivePower(),  Line::TOTAL);
const ObisData ObisData::NegativeActivePowerL1    (0, 22, 4, 0, MeasurementType::EmeterNegativeActivePower(),  Line::L1);
const ObisData ObisData::NegativeActivePowerL2    (0, 42, 4, 0, MeasurementType::EmeterNegativeActivePower(),  Line::L2);
const ObisData ObisData::NegativeActivePowerL3    (0, 62, 4, 0, MeasurementType::EmeterNegativeActivePower(),  Line::L3);
const ObisData ObisData::NegativeActiveEnergyTotal(0,  2, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(), Line::TOTAL);
const ObisData ObisData::NegativeActiveEnergyL1   (0, 22, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(), Line::L1);
const ObisData ObisData::NegativeActiveEnergyL2   (0, 42, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(), Line::L2);
const ObisData ObisData::NegativeActiveEnergyL3   (0, 62, 8, 0, MeasurementType::EmeterNegativeActiveEnergy(), Line::L3);
const ObisData ObisData::PowerFactorTotal         (0, 13, 4, 0, MeasurementType::EmeterPowerFactor(),          Line::TOTAL);
const ObisData ObisData::PowerFactorL1            (0, 33, 4, 0, MeasurementType::EmeterPowerFactor(),          Line::L1);
const ObisData ObisData::PowerFactorL2            (0, 53, 4, 0, MeasurementType::EmeterPowerFactor(),          Line::L2);
const ObisData ObisData::PowerFactorL3            (0, 73, 4, 0, MeasurementType::EmeterPowerFactor(),          Line::L3);
const ObisData ObisData::VoltageL1                (0, 32, 4, 0, MeasurementType::EmeterVoltage(),              Line::L1);
const ObisData ObisData::VoltageL2                (0, 52, 4, 0, MeasurementType::EmeterVoltage(),              Line::L2);
const ObisData ObisData::VoltageL3                (0, 72, 4, 0, MeasurementType::EmeterVoltage(),              Line::L3);
const ObisData ObisData::CurrentL1                (0, 31, 4, 0, MeasurementType::EmeterCurrent(),              Line::L1);
const ObisData ObisData::CurrentL2                (0, 51, 4, 0, MeasurementType::EmeterCurrent(),              Line::L2);
const ObisData ObisData::CurrentL3                (0, 71, 4, 0, MeasurementType::EmeterCurrent(),              Line::L3);
const ObisData ObisData::SignedActivePowerTotal   (0, 16, 7, 0, MeasurementType::EmeterSignedActivePower(),    Line::TOTAL); 
const ObisData ObisData::SignedActivePowerL1      (0, 36, 7, 0, MeasurementType::EmeterSignedActivePower(),    Line::L1); 
const ObisData ObisData::SignedActivePowerL2      (0, 56, 7, 0, MeasurementType::EmeterSignedActivePower(),    Line::L2);
const ObisData ObisData::SignedActivePowerL3      (0, 76, 7, 0, MeasurementType::EmeterSignedActivePower(),    Line::L3);
