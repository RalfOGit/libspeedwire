#ifndef __MEASUREMENT_HPP__
#define __MEASUREMENT_HPP__

#include <cstdint>
#include <string>
#include <time.h>


enum class Direction {
    POSITIVE,
    NEGATIVE,
    SIGNED,
    NO_DIRECTION
};
std::string toString(const Direction direction);

enum class Line {
    TOTAL,
    L1,
    L2,
    L3,
    MPP_TOTAL,
    MPP1,
    MPP2,
    LOSS_TOTAL,
    DEVICE_OK,
    RELAY_ON,
    NO_LINE
};
std::string toString(const Line line);

enum class Quantity {
    POWER,
    ENERGY,
    POWER_FACTOR,
    CURRENT,
    VOLTAGE,
    STATUS,
    EFFICIENCY,
    NO_QUANTITY
};
std::string toString(const Quantity quantity);
bool isInstantaneous(const Quantity quantity);

enum class Type {
    ACTIVE,
    REACTIVE,
    APPARENT,
    VERSION,
    END_OF_DATA,
    NO_TYPE
};
std::string toString(const Type type);


class MeasurementType {
public:
    std::string name;
    std::string unit;       // measurement unit after applying the divisor, e.g. W, kWh
    unsigned long divisor;  // divide value by divisor to obtain floating point measurements in the given unit
    bool instaneous;        // true for power measurements, false for energy measurements
    Direction direction;    // true for consumed from grid, false for provided to the grid
    Quantity quantity;
    Type type;

    MeasurementType(const Direction direction, const Type type, const Quantity quantity, 
                    const std::string &unit, const unsigned long divisor);

    std::string getFullName(const Line line) const;


    // pre-defined instances for emeter measurement types
    // these definitions are used by static initializers; to avoid static initialization ordering issues, define them as methods
    static MeasurementType EmeterPositiveActivePower    (void) { return MeasurementType(Direction::POSITIVE,     Type::ACTIVE,      Quantity::POWER,        "W",         10); }
    static MeasurementType EmeterPositiveActiveEnergy   (void) { return MeasurementType(Direction::POSITIVE,     Type::ACTIVE,      Quantity::ENERGY,       "kWh",  3600000); }
    static MeasurementType EmeterNegativeActivePower    (void) { return MeasurementType(Direction::NEGATIVE,     Type::ACTIVE,      Quantity::POWER,        "W",         10); }
    static MeasurementType EmeterNegativeActiveEnergy   (void) { return MeasurementType(Direction::NEGATIVE,     Type::ACTIVE,      Quantity::ENERGY,       "kWh",  3600000); }
    static MeasurementType EmeterPositiveApparentPower  (void) { return MeasurementType(Direction::POSITIVE,     Type::APPARENT,    Quantity::POWER,        "VA",        10); }
    static MeasurementType EmeterPositiveApparentEnergy (void) { return MeasurementType(Direction::POSITIVE,     Type::APPARENT,    Quantity::ENERGY,       "VAh",  3600000); }
    static MeasurementType EmeterNegativeApparentPower  (void) { return MeasurementType(Direction::NEGATIVE,     Type::APPARENT,    Quantity::POWER,        "Var",       10); }
    static MeasurementType EmeterNegativeApparentEnergy (void) { return MeasurementType(Direction::NEGATIVE,     Type::APPARENT,    Quantity::ENERGY,       "Varh", 3600000); }
    static MeasurementType EmeterPositiveReactivePower  (void) { return MeasurementType(Direction::POSITIVE,     Type::REACTIVE,    Quantity::POWER,        "W",         10); }
    static MeasurementType EmeterPositiveReactiveEnergy (void) { return MeasurementType(Direction::POSITIVE,     Type::REACTIVE,    Quantity::ENERGY,       "kWh",  3600000); }
    static MeasurementType EmeterNegativeReactivePower  (void) { return MeasurementType(Direction::NEGATIVE,     Type::REACTIVE,    Quantity::POWER,        "W",         10); }
    static MeasurementType EmeterNegativeReactiveEnergy (void) { return MeasurementType(Direction::NEGATIVE,     Type::REACTIVE,    Quantity::ENERGY,       "kWh",  3600000); }
    static MeasurementType EmeterSignedActivePower      (void) { return MeasurementType(Direction::SIGNED,       Type::ACTIVE,      Quantity::POWER,        "W",         10); }
    static MeasurementType EmeterPowerFactor            (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::POWER_FACTOR, "phi",     1000); }
    static MeasurementType EmeterVoltage                (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::VOLTAGE,      "V",       1000); }
    static MeasurementType EmeterCurrent                (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::CURRENT,      "A",       1000); }
    static MeasurementType EmeterSoftwareVersion        (void) { return MeasurementType(Direction::NO_DIRECTION, Type::VERSION,     Quantity::NO_QUANTITY,  "",           1); }
    static MeasurementType EmeterEndOfData              (void) { return MeasurementType(Direction::NO_DIRECTION, Type::END_OF_DATA, Quantity::NO_QUANTITY,  "",           1); }
//    static MeasurementType EmeterStatus                 (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,  Quantity::STATUS,       "-",          1); }

    // pre-defined instances for inverter measurement types
    static MeasurementType InverterPower                (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::POWER,         "W",          1); }
    static MeasurementType InverterVoltage              (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::VOLTAGE,       "V",        100); }
    static MeasurementType InverterCurrent              (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::CURRENT,       "A",       1000); }
    static MeasurementType InverterStatus               (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::STATUS,         "",          1); }
    static MeasurementType InverterRelay                (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::STATUS,         "",          1); }
    static MeasurementType InverterEfficiency           (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::EFFICIENCY,    "%",          1); }
    static MeasurementType InverterLoss                 (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE, Quantity::POWER,         "W",          1); }
};


class MeasurementValue {
public:
    double       value;         // the current measurement
    std::string  value_string;  // the current measurement, if it is not a numeric value (e.g. software version, etc)
    uint32_t     timer;         // the current timestamp
    uint32_t     elapsed;       // time elapsed from previous timestamp to current timestamp
    double       sumValue;      // the sum of previous and current measurements
    unsigned int counter;       // the number of measurements included in sumValue
    bool         initial;

    MeasurementValue(void);
    void setValue(int32_t  raw_value, unsigned long divisor);
    void setValue(uint32_t raw_value, unsigned long divisor);
    void setValue(uint64_t raw_value, unsigned long divisor);
    void setValue(const std::string& raw_value);
    void setTimer(uint32_t timer);
};

#endif
