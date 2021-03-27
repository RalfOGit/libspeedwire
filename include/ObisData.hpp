#ifndef __OBISDATA_HPP__
#define __OBISDATA_HPP__

#include <cstdint>
#include <stdio.h>
#include <string>
#include <array>
#include <Measurement.hpp>


/**
 *  Class holding an obis data definition used inside speedwire emeter packets 
 */
class ObisType {
public:
    uint8_t channel;
    uint8_t index;
    uint8_t type;
    uint8_t tariff;
 
    ObisType(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff);

    bool equals(const ObisType &other) const;

    std::string toString(void) const;
    void print(const uint32_t value, FILE *file) const;
    void print(const uint64_t value, FILE *file) const;

    std::array<uint8_t, 8> toByteArray(void) const;
};


/**
 *  Class holding an emeter measurement together with its corresponding obis data definition and measurement type definition
 */
class ObisData : public ObisType {
public:
    MeasurementType   measurementType;
    MeasurementValue *measurementValue;
    Line              line;
    std::string       description;

    ObisData(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff, const MeasurementType &measurementType, const Line &line);
    ObisData(const ObisData &rhs);
    ObisData &operator=(const ObisData &rhs);
    
    ~ObisData(void);

    bool equals(const ObisType &other) const;

    void print(FILE *file) const;

    // pre-defined instances
    static const ObisData PositiveActivePowerTotal;
    static const ObisData PositiveActivePowerL1;
    static const ObisData PositiveActivePowerL2;
    static const ObisData PositiveActivePowerL3;
    static const ObisData PositiveActiveEnergyTotal;
    static const ObisData PositiveActiveEnergyL1;
    static const ObisData PositiveActiveEnergyL2;
    static const ObisData PositiveActiveEnergyL3;
    static const ObisData NegativeActivePowerTotal;
    static const ObisData NegativeActivePowerL1;
    static const ObisData NegativeActivePowerL2;
    static const ObisData NegativeActivePowerL3;
    static const ObisData NegativeActiveEnergyTotal;
    static const ObisData NegativeActiveEnergyL1;
    static const ObisData NegativeActiveEnergyL2;
    static const ObisData NegativeActiveEnergyL3;
    static const ObisData PowerFactorTotal;
    static const ObisData PowerFactorL1;
    static const ObisData PowerFactorL2;
    static const ObisData PowerFactorL3;
    static const ObisData VoltageL1;
    static const ObisData VoltageL2;
    static const ObisData VoltageL3;
    static const ObisData CurrentL1;
    static const ObisData CurrentL2;
    static const ObisData CurrentL3;
    static const ObisData SignedActivePowerTotal;
    static const ObisData SignedActivePowerL1;
    static const ObisData SignedActivePowerL2;
    static const ObisData SignedActivePowerL3;
};

#endif
