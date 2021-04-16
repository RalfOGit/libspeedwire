#ifndef __SPEEDWIREDATA_HPP__
#define __SPEEDWIREDATA_HPP__

#include <cstdint>
#include <string>
#include <stdio.h>
#include <map>
#include <Measurement.hpp>


/**
 *  Class holding raw data from the speedwire inverter reply packet
 */
class SpeedwireRawData {
public:
    uint32_t command;                   // command code
    uint32_t id;                        // register id
    uint8_t  conn;                      // connector id (mpp #1, mpp #2, ac #1)
    uint8_t  type;                      // unknown type
    time_t   time;                      // timestamp
    uint8_t  data[40];                  // payload data
    size_t   data_size;                 // payload data size in bytes

    SpeedwireRawData(const uint32_t command, const uint32_t id, const uint8_t conn, const uint8_t type, const time_t time, const void* const data, const size_t data_size);

    bool equals(const SpeedwireRawData& other) const;
    bool isSameSignature(const SpeedwireRawData& other) const;

    uint32_t toKey(void) const { return id | conn; }

    std::string toString(void) const;
    void print(uint32_t value, FILE* file) const;
    void print(uint64_t value, FILE* file) const;
};


/**
 *  Class holding data from the speedwire inverter reply packet, enriched by measurement type information 
 *  and the interpreted measurement value
 */
class SpeedwireData : public SpeedwireRawData {
public:
    MeasurementType   measurementType;
    MeasurementValue* measurementValue;
    Wire              wire;
    std::string       description;

    SpeedwireData(const uint32_t command, const uint32_t id, const uint8_t conn, const uint8_t type, const time_t time, const void* data, const size_t data_size,
                        const MeasurementType& mType, const Wire wire);
    SpeedwireData(const SpeedwireData& rhs);
    SpeedwireData(void);
    SpeedwireData& operator=(const SpeedwireData& rhs);

    ~SpeedwireData(void);

    bool consume(const SpeedwireRawData& data);

    void print(FILE* file) const;


    // pre-defined instances
    static const SpeedwireData InverterPowerMPP1;
    static const SpeedwireData InverterPowerMPP2;
    static const SpeedwireData InverterVoltageMPP1;
    static const SpeedwireData InverterVoltageMPP2;
    static const SpeedwireData InverterCurrentMPP1;
    static const SpeedwireData InverterCurrentMPP2;
    static const SpeedwireData InverterPowerL1;
    static const SpeedwireData InverterPowerL2;
    static const SpeedwireData InverterPowerL3;
    static const SpeedwireData InverterVoltageL1;       // L1 -> N
    static const SpeedwireData InverterVoltageL2;       // L2 -> N
    static const SpeedwireData InverterVoltageL3;       // L3 -> N
    static const SpeedwireData InverterVoltageL1toL2;   // L1 -> L2
    static const SpeedwireData InverterVoltageL2toL3;   // L2 -> L3
    static const SpeedwireData InverterVoltageL3toL1;   // L3 -> L1
    static const SpeedwireData InverterCurrentL1;
    static const SpeedwireData InverterCurrentL2;
    static const SpeedwireData InverterCurrentL3;
    static const SpeedwireData InverterStatus;
    static const SpeedwireData InverterRelay;

    static const SpeedwireData InverterPowerDCTotal;
    static const SpeedwireData InverterPowerACTotal;
    static const SpeedwireData InverterPowerLoss;
    static const SpeedwireData InverterPowerEfficiency;
};


/**
 *  Interface to be implemented by the consumer of speedwire inverter reply data
 */
class SpeedwireConsumer {
public:
    virtual void consume(const SpeedwireData& element) = 0;
};


/**
 *  Class implementing a query map for speedwire inverter reply data
 */
class SpeedwireDataMap : public std::map<uint32_t, SpeedwireData> {
public:
    // add a new element to the map of speedwire inverter reply data elements
    void add(const SpeedwireData& map) { operator[](toKey(map)) = map; }

    // find a speedwire inverter reply data map element by the given key and add its measurement value to the target element
    void addValueToTarget(uint32_t key, SpeedwireData& target) const {
        auto iterator = find(key);
        if (iterator != end()) {
            target.measurementValue->value += iterator->second.measurementValue->value;
            target.measurementValue->timer  = iterator->second.measurementValue->timer;
            target.time = iterator->second.time;
        }
    }

    // combine id and conn to form a query map key
    static uint32_t toKey(const SpeedwireRawData& key) { return key.toKey(); }
};

#endif
