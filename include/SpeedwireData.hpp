#ifndef __LIBSPEEDWIRE_SPEEDWIREDATA_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDATA_HPP__

#include <cstdint>
#include <string>
#include <stdio.h>
#include <map>
#include <Measurement.hpp>
#include <MeasurementType.hpp>
#include <MeasurementValues.hpp>

namespace libspeedwire {

    /**
     *  Enumeration of data types used in speedwire inverter reply packets.
     */
    enum class SpeedwireDataType : uint8_t {
        UnsignedLong = 0x00,
        Status       = 0x08,
        String       = 0x10,
        Float        = 0x20,
        SignedLong   = 0x40
    };

    //! Convert SpeedwireDataType to a string
    std::string toString(SpeedwireDataType type);


    /**
     *  Class holding raw data from the speedwire inverter reply packet.
     */
    class SpeedwireRawData {
    public:
        uint32_t command;                   //!< command code
        uint32_t id;                        //!< register id
        uint8_t  conn;                      //!< connector id (mpp #1, mpp #2, ac #1)
        SpeedwireDataType type;             //!< type
        time_t   time;                      //!< timestamp
        uint8_t  data[40];                  //!< payload data
        size_t   data_size;                 //!< payload data size in bytes

        SpeedwireRawData(const uint32_t command, const uint32_t id, const uint8_t conn, const SpeedwireDataType type, const time_t time, const void* const data, const size_t data_size);
        SpeedwireRawData(void);

        bool equals(const SpeedwireRawData& other) const;
        bool isSameSignature(const SpeedwireRawData& other) const;

        /** Return key for this instance. The key is formed by combining id and conn.
         *  @return The key for this instance */
        uint32_t toKey(void) const { return id | conn; }

        std::string toString(void) const;
        std::string toString(uint32_t value) const;
        std::string toString(uint64_t value) const;
    };


    /**
     *  Class holding data from the speedwire inverter reply packet, enriched by measurement type information and the interpreted measurement value.
     */
    class SpeedwireData : public SpeedwireRawData, public Measurement {
    public:

        SpeedwireData(const uint32_t command, const uint32_t id, const uint8_t conn, const SpeedwireDataType type, const time_t time,
            const void* data, const size_t data_size, const MeasurementType& mType, const Wire wire);
        SpeedwireData(void);

        bool consume(const SpeedwireRawData& data);

        std::string toString(void) const;

        // pre-defined static instances
        static const SpeedwireData InverterPowerMPP1;              //!< Power on direct current inverter input MPP1
        static const SpeedwireData InverterPowerMPP2;              //!< Power on direct current inverter input MPP2
        static const SpeedwireData InverterVoltageMPP1;            //!< Voltage on direct current inverter input MPP1
        static const SpeedwireData InverterVoltageMPP2;            //!< Voltage on direct current inverter input MPP2
        static const SpeedwireData InverterCurrentMPP1;            //!< Current on direct current inverter input MPP1
        static const SpeedwireData InverterCurrentMPP2;            //!< Current on direct current inverter input MPP1
        static const SpeedwireData InverterPowerL1;                //!< Power on alternating current inverter output phase L1
        static const SpeedwireData InverterPowerL2;                //!< Power on alternating current inverter output phase L2
        static const SpeedwireData InverterPowerL3;                //!< Power on alternating current inverter output phase L3
        static const SpeedwireData InverterVoltageL1;              //!< Voltage on alternating current inverter output phase L1, measured between L1 and N
        static const SpeedwireData InverterVoltageL2;              //!< Voltage on alternating current inverter output phase L2, measured between L2 and N
        static const SpeedwireData InverterVoltageL3;              //!< Voltage on alternating current inverter output phase L3, measured between L3 and N
        static const SpeedwireData InverterVoltageL1toL2;          //!< Voltage on alternating current inverter output phase L1, measured between L1 and L2
        static const SpeedwireData InverterVoltageL2toL3;          //!< Voltage on alternating current inverter output phase L1, measured between L2 and L3
        static const SpeedwireData InverterVoltageL3toL1;          //!< Voltage on alternating current inverter output phase L1, measured between L3 and L1
        static const SpeedwireData InverterCurrentL1;              //!< Current on alternating current inverter output phase L1
        static const SpeedwireData InverterCurrentL2;              //!< Current on alternating current inverter output phase L2
        static const SpeedwireData InverterCurrentL3;              //!< Current on alternating current inverter output phase L3
        static const SpeedwireData InverterStatus;                 //!< Inverter operation status
        static const SpeedwireData InverterRelay;                  //!< Grid relay status

        static const SpeedwireData InverterPowerDCTotal;           //!< Total power on direct current inverter inputs MPP1 + MPP2
        static const SpeedwireData InverterPowerACTotal;           //!< Total power on alternating current inverter outputs L1 + L2 + L3
        static const SpeedwireData InverterPowerLoss;              //!< Total power loss
        static const SpeedwireData InverterPowerEfficiency;        //!< Total power efficiency

        static const SpeedwireData HouseholdPowerTotal;            //!< Total power consumption of the household
        static const SpeedwireData HouseholdIncomeTotal;           //!< Income generated
        static const SpeedwireData HouseholdIncomeFeedIn;          //!< Income generated from grid feed-in
        static const SpeedwireData HouseholdIncomeSelfConsumption; //!< Income generated from self-consumption

    };


    /**
     *  Class implementing a query map for speedwire inverter reply data.
     *  The class extends std::map<uint32_t, SpeedwireData> and adds functionality to deal with keys derived from
        a SpeedwireRawData instance.
     */
    class SpeedwireDataMap : public std::map<uint32_t, SpeedwireData> {
    public:
        /**
         *  Add a new element to the map of speedwire inverter reply data elements.
         *  @param element The SpeedwireData element added to the map
         */
        void add(const SpeedwireData& element) { operator[](element.toKey()) = element; }

        /**
         *  Find a speedwire inverter reply data map element by the given key and add its measurement value to the target element.
         *  @param key The search key
         *  @param target The target SpeedwireData element to be modified with the value of the element referenced by the search key
         */
        //void addValueToTarget(uint32_t key, SpeedwireData& target) const {
        //    auto iterator = find(key);
        //    if (iterator != end()) {
        //        target.measurementValue.value += iterator->second.measurementValue.value;
        //        target.measurementValue.timer = iterator->second.measurementValue.timer;
        //        target.time = iterator->second.time;
        //    }
        //}
    };

}   // namespace libspeedwire

#endif
