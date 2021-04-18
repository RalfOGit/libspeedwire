#ifndef __MEASUREMENT_HPP__
#define __MEASUREMENT_HPP__

#include <cstdint>
#include <string>
#include <time.h>

/**
 *  Enumeration describing the direction of energy flows.
 */
enum class Direction {
    POSITIVE,           //!< Positive direction - energy is consumed from the grid
    NEGATIVE,           //!< Negative direction - energy is provided to the grid
    SIGNED,             //!< Positive and negative direction expressed by a signed value
    NO_DIRECTION        //!< Direction is not applicable
};

//! Convert Direction to a string
std::string toString(const Direction direction);


/**
 *  Enumeration describing the wire for the energy flow.
 *  Totals and status values are considered as separate wires.
 */
enum class Wire {
    TOTAL,              //!< Total of L1+L2+L3 of three-phase alternating current
    L1,                 //!< Phase L1 of three-phase alternating current wire
    L2,                 //!< Phase L2 of three-phase alternating current wire
    L3,                 //!< Phase L3 of three-phase alternating current wire
    MPP_TOTAL,          //!< Total of MPP1+MPP2 direct current
    MPP1,               //!< MPP1 direct current wire
    MPP2,               //!< MPP2 direct current wire
    LOSS_TOTAL,         //!< Total of L1+L2+L3 of three-phase alternating current minus total of MPP1+MPP2 direct current
    DEVICE_OK,          //!< Device OK status
    RELAY_ON,           //!< Grid relay switched on
    NO_WIRE             //!< Wire is not applicable
};

//! Convert Wire to a string
std::string toString(const Wire line);


/**
 *  Enumeration describing the physical quantity.
 */
enum class Quantity {
    POWER,              //!< Electrical power
    ENERGY,             //!< Electrical energy
    POWER_FACTOR,       //!< Power factor
    CURRENT,            //!< Electrical current
    VOLTAGE,            //!< Electrical voltage
    STATUS,             //!< Device status
    EFFICIENCY,         //!< Energy efficiency
    NO_QUANTITY         //!< Quantity is not applicable
};

//! Convert Quantity to a string
std::string toString(const Quantity quantity);

//! Returns if the given Quantity is an instantaneous quantity like power, voltage, current, ...; contrary to energy which is an accumulated quantity like energy
bool isInstantaneous(const Quantity quantity);


/**
 *  Enumeration describing the type of the measurement.
 */
enum class Type {
    ACTIVE,              //!< Electrical active power or energy
    REACTIVE,            //!< Electrical reactive power or energy
    APPARENT,            //!< Electrical apparent power or energy
    VERSION,             //!< Software version
    END_OF_DATA,         //!< End of data marker
    NO_TYPE              //!< Type is not applicable
};

//! Convert Type to a string
std::string toString(const Type type);


/**
 *  Class encapsulating fixed properties of a measurement type.
 */
class MeasurementType {
public:
    std::string name;       //!< Printable name constructed from the Direction, Quantity and Type properties below
    std::string unit;       //!< Measurement unit after applying the divisor, e.g. W, kWh
    unsigned long divisor;  //!< Divide value by divisor to obtain floating point measurements in the given unit
    bool instaneous;        //!< True for power measurements, false for energy measurements
    Direction direction;    //!< Direction of the energy flow
    Quantity quantity;      //!< Physical quantity of the measurement
    Type type;              //!< Type of the measurement

    MeasurementType(const Direction direction, const Type type, const Quantity quantity, const std::string &unit, const unsigned long divisor);

    std::string getFullName(const Wire line) const;


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

    // pre-defined instances for inverter measurement types
    static MeasurementType InverterPower                (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::POWER,        "W",          1); }
    static MeasurementType InverterVoltage              (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::VOLTAGE,      "V",        100); }
    static MeasurementType InverterCurrent              (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::CURRENT,      "A",       1000); }
    static MeasurementType InverterStatus               (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::STATUS,        "",          1); }
    static MeasurementType InverterRelay                (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::STATUS,        "",          1); }
    static MeasurementType InverterEfficiency           (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::EFFICIENCY,   "%",          1); }
    static MeasurementType InverterLoss                 (void) { return MeasurementType(Direction::NO_DIRECTION, Type::NO_TYPE,     Quantity::POWER,        "W",          1); }
};


/**
 *  Class encapsulating the value and other variable properties of a measurement.
 */
class MeasurementValue {
public:
    double       value;         //!< Value of the current, i.e. most recent measurement
    std::string  value_string;  //!< Value of the current, i.e. most recent measurement, if it is not a numeric value (e.g. software version, etc)
    uint32_t     timer;         //!< The current, i.e. most recent timestamp
    uint32_t     elapsed;       //!< Time elapsed from previous timestamp to current timestamp
    double       sumValue;      //!< The sum of previous and current measurements
    unsigned int counter;       //!< The number of measurements included in sumValue
    bool         initial;       //!< Flag indicating that this instance is new

    MeasurementValue(void);
    void setValue(int32_t  raw_value, unsigned long divisor);
    void setValue(uint32_t raw_value, unsigned long divisor);
    void setValue(uint64_t raw_value, unsigned long divisor);
    void setValue(const std::string& raw_value);
    void setTimer(uint32_t timer);
};

#endif
