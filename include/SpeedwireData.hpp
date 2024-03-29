#ifndef __LIBSPEEDWIRE_SPEEDWIREDATA_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDATA_HPP__

#include <cstdint>
#include <string>
#include <stdio.h>
#include <map>
#include <SpeedwireCommand.hpp>
#include <Measurement.hpp>
#include <MeasurementType.hpp>
#include <MeasurementValues.hpp>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireStatus.hpp>

namespace libspeedwire {

    //  From the Modbus documentation: SMA-Modbus-general-TI-en-10.pdf
    //  3.5.7 SMA Data Types and NaN Values
    //      The following table shows the data types used in the SMA Modbus profile and compares these to
    //      possible NaN values.The SMA data types are listed in the assignment tables in the Type column.
    //      The SMA data types describe the data widths of the assigned values :
    //  Type        Explanation                             NaN value
    //      S16     A signed word (16-bit).                 0x8000
    //      S32     A signed double word (32-bit).          0x8000 0000
    //      STR32   32 byte data field, in UTF8 format.     ZERO
    //      U16     A word (16-bit).                        0xFFFF
    //      U32     A double word (32-bit).                 0xFFFF FFFF
    //      U32     For status values, only the lower 24 bits 
    //              of a double word (32-bit) are used.     0x00FF FFFD
    //      U64     A quadruple word (64-bit).              0xFFFF FFFF FFFF FFFF
    //
    // 0x00FF FFFE is apparently used as a "no value" or "end of data" word for status values

    /**
     *  Enumeration of data types used in speedwire inverter reply packets.
     */
    enum class SpeedwireDataType : uint8_t {
        Unsigned32 = 0x00,
        Status32   = 0x08,
        String32   = 0x10,
        Float      = 0x20,  // likely unused
        Signed32   = 0x40,
        Event      = 0xf0,  // arbitrary custom definition, not defined by SMA
        Yield      = 0xf8,  // arbitrary custom definition, not defined by SMA

        TypeMask   = 0xf8,  // to mask data type bits, like Unsigned32 ... Yield
        FlagMask   = 0x07,  // to mask flag bits, like write flag, ...

        WriteFlag  = 0x02   // signifies a data type used for a write operation
    };

    //! Convert SpeedwireDataType to a string
    std::string toString(SpeedwireDataType type);

    static SpeedwireDataType operator|(SpeedwireDataType lhs, SpeedwireDataType rhs) { return (SpeedwireDataType)(((uint8_t)lhs) | ((uint8_t)rhs)); }
    static SpeedwireDataType operator&(SpeedwireDataType lhs, SpeedwireDataType rhs) { return (SpeedwireDataType)(((uint8_t)lhs) & ((uint8_t)rhs)); }
    static SpeedwireDataType operator~(SpeedwireDataType rhs) { return (SpeedwireDataType)~((uint8_t)rhs); }
    static bool operator==(SpeedwireDataType lhs, SpeedwireDataType rhs) { return (((uint8_t)lhs) == ((uint8_t)rhs)); }


    /**
     *  Class holding raw data from the speedwire inverter reply packet.
     */
    class SpeedwireRawData {
    public:
        Command  command;        //!< command code
        uint32_t id;             //!< register id
        uint8_t  conn;           //!< connector id (mpp #1, mpp #2, ac #1)
        SpeedwireDataType type;  //!< type
        time_t   time;           //!< timestamp
        uint8_t  data[44];       //!< payload data
        size_t   data_size;      //!< payload data size in bytes

        SpeedwireRawData(const Command command, const uint32_t id, const uint8_t conn, const SpeedwireDataType type, const time_t time, const void* const data, const size_t data_size);
        SpeedwireRawData(void);

        bool equals(const SpeedwireRawData& other) const;
        bool isSameSignature(const SpeedwireRawData& other) const;

        /** Return key for this instance. The key is formed by combining id and conn.
         *  @return The key for this instance
         */
        uint32_t toKey(void) const { return id | conn; }

        std::string toHexString(void) const;
        std::string toString(void) const;

        size_t getNumberOfValues(void) const;
        size_t getNumberOfSignificantValues(void) const;
    };


    /**
     *  Wrapper class to simplify access to SpeedwireRawData of type Unsigned32
     */
    class SpeedwireRawDataUnsigned32 {
    protected:
        const SpeedwireRawData& base;

    public:
        static const size_t value_size = 4u;
        static const uint32_t nan = 0xffffffff;
        static const uint32_t eod = 0xfffffffe;

        SpeedwireRawDataUnsigned32(const SpeedwireRawData& raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        bool isNanValue(uint32_t value) const { return (value == nan); }
        bool isEoDValue(uint32_t value) const { return (value == eod); }
        uint32_t getValue(size_t pos) const { return SpeedwireByteEncoding::getUint32LittleEndian(base.data + pos * value_size); }
        std::vector<uint32_t> getValues(void) const;
        double convertValueToDouble(uint32_t value) const { return (double)value; }

        std::string convertValueToString(uint32_t value, bool hex) const {
            if (value == nan) { return "NaN"; }
            if (value == eod) { return "EoD"; }
            char byte[32];
            snprintf(byte, sizeof(byte), (hex ? "0x%08lx" : "%lu"), (unsigned long)value);
            return std::string(byte);
        }

        bool isValueWithRange(void) const { return (base.getNumberOfValues() == 8 && base.getNumberOfSignificantValues() == 4); }
        bool isRevisionOrSerial(void) const { return (isValueWithRange() && getValue(0) == 0 && (isEoDValue(getValue(2)) || isNanValue(getValue(2))) && !isNanValue(getValue(4))); }
        std::string toValueWithRangeString(void) const;
        std::string toRevisionOrSerialString(void) const;
    };


    /**
     *  Wrapper class to simplify access to SpeedwireRawData of type Signed32
     */
    class SpeedwireRawDataSigned32 {
    protected:
        const SpeedwireRawData& base;

    public:
        static const size_t value_size = 4u;
        static const int32_t nan = 0x80000000;

        SpeedwireRawDataSigned32(const SpeedwireRawData& raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        bool isNanValue(int32_t value) const { return (value == nan); }
        int32_t getValue(size_t pos) const { return (int32_t)SpeedwireByteEncoding::getUint32LittleEndian(base.data + pos * value_size); }
        std::vector<int32_t> getValues(void) const;
        double convertValueToDouble(int32_t value) const { return (double)value; }

        std::string convertValueToString(int32_t value, bool hex) const {
            if (value == nan) { return "NaN"; }
            char byte[32];
            snprintf(byte, sizeof(byte), (hex ? "0x%08lx" : "%ld"), (unsigned long)value);
            return std::string(byte);
        }

        bool isValueWithRange(void) const { return (base.getNumberOfValues() == 8 && base.getNumberOfSignificantValues() == 4); }
        std::string toValueWithRangeString(void) const;
    };


    /**
     *  Wrapper class to simplify access to SpeedwireRawData of type Status32
     */
    class SpeedwireRawDataStatus32 {
    protected:
        const SpeedwireRawData &base;

    public:
        static const size_t   value_size = 4u;
        static const uint32_t value_mask = 0x00ffffff;
        static const uint32_t marker_mask = 0xff000000;
        static const uint32_t nan = 0x00fffffd;
        static const uint32_t eod = 0x00fffffe;
        static const uint32_t sel = 0x01000000;

        SpeedwireRawDataStatus32(const SpeedwireRawData &raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        bool isNanValue(uint32_t value) const { return ((value & value_mask) == nan); }
        bool isEoDValue(uint32_t value) const { return ((value & value_mask) == eod); }
        bool isM1Value(uint32_t value) const { return ((value & value_mask) == value_mask); }
        bool isNumber(uint32_t value) const { return !(isNanValue(value) || isEoDValue(value) || isM1Value(value)); }
        uint32_t getValue(size_t pos, bool masked = false) const { return SpeedwireByteEncoding::getUint32LittleEndian(base.data + pos * value_size) & (masked ? value_mask : (marker_mask |value_mask)); }

        /** Get the index of the data value marked with 0x01000000; this is the selected value in the list of values */
        size_t getSelectionIndex(void) const {
            size_t num_values = getNumberOfValues();
            for (size_t i = 0; i < num_values; ++i) {
                uint32_t svalue = getValue(i);
                if ((svalue & marker_mask) == sel) {
                    return i;
                }
            }
            return (size_t)-1;
        }

        double convertValueToDouble(uint32_t value) const {
            return (double)(value & value_mask);
        }

        std::string convertValueToString(uint32_t value) const {
            if (value == nan) { return "NaN"; }
            if (value == (sel | nan)) { return "->NaN"; }
            if (value == eod) { return "EoD"; }
            const SpeedwireStatus& status = SpeedwireStatusMap::getFromGlobalMap(value & value_mask);
            if (status != SpeedwireStatus::NOTFOUND()) {
                if (value == (sel | status.value)) { return "->" + status.name; }
                return status.name;
            }
            char byte[32];
            snprintf(byte, sizeof(byte), "0x%08lx", (unsigned long)value);
            return std::string(byte);
        }

        std::vector<uint32_t> getValues(void) const {
            std::vector<uint32_t> values;
            size_t sel = getSelectionIndex();
            if (sel != (size_t)-1) {
                values.push_back(getValue(sel, true));
            }
            return values;
        }
    };


    /**
     *  Wrapper class to simplify access to SpeedwireRawData of type String32
     */
    class SpeedwireRawDataString32 {
        protected:
            const SpeedwireRawData& base;

        public:
        static const size_t value_size = 32u;

        SpeedwireRawDataString32(const SpeedwireRawData& raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        std::string getValue(size_t pos) const { return std::string((char*)base.data + pos * value_size, base.data_size - pos * value_size); }

        std::vector<std::string> getValues(void) const {
            std::vector<std::string> values;
            for (size_t i = 0; i < getNumberOfValues(); ++i) {
                values.push_back(getValue(i));
            }
            return values;
        }

        std::string getHexValue(size_t pos) const {
            std::string result("0x");
            for (size_t i = pos * value_size; i < (pos + 1) * value_size; ++i) {
                static const char hex[17] = "0123456789abcdef";
                result.append(1, hex[(base.data[i] >> 4) & 0x0f]);
                result.append(1, hex[ base.data[i]       & 0x0f]);
            }
            return result;
        }
    };


    /**
     *  Wrapper class to simplify access to SpeedwireRawData of type Yield.
     */
    class SpeedwireRawDataYield {
    protected:
        const SpeedwireRawData& base;

    public:
        static const size_t value_size = 8u;

        class YieldValue {
        public:
            time_t    epoch_time;
            uint64_t  yield_value;
            YieldValue(time_t time, uint64_t value) : epoch_time(time), yield_value(value) {}
        };

        SpeedwireRawDataYield(const SpeedwireRawData& raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        YieldValue getValue(size_t pos) const { return YieldValue(base.time, SpeedwireByteEncoding::getUint64LittleEndian(base.data + pos * value_size)); }

        std::vector<YieldValue> getValues(void) const {
            std::vector<YieldValue> values;
            for (size_t i = 0; i < getNumberOfValues(); ++i) {
                values.push_back(getValue(i));
            }
            return values;
        }

        std::string convertValueToString(const YieldValue& value, bool hex) const {
            std::string result = LocalHost::unixEpochTimeInMsToString(SpeedwireTime::convertInverterTimeToUnixEpochTime((uint32_t)value.epoch_time));

            char str[64];
            snprintf(str, sizeof(str), (hex ? " 0x%016llx" : " %llu"), value.yield_value);
            result.append(str);
            return result;
        }
    };


    /**
     *  Wrapper class to simplify access to SpeedwireRawData of type Event.
     */
    class SpeedwireRawDataEvent {
    protected:
        const SpeedwireRawData& base;

    public:
        static const size_t value_size = 44u;

        class EventValue {
        public:
            time_t    epoch_time;
            uint16_t  entry_id;
            uint16_t  susy_id;
            uint32_t  serial_number;
            uint16_t  event_id;
            uint8_t   marker_1;
            uint8_t   marker_2;
            uint32_t  value_1;
            uint32_t  value_2;
            uint32_t  event_tag_id;
            uint32_t  value[5];
            EventValue(time_t time, uint8_t* data, size_t data_size);
        };

        SpeedwireRawDataEvent(const SpeedwireRawData& raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        EventValue getValue(size_t pos) const { return EventValue(base.time, (uint8_t*)base.data + pos * value_size, value_size); }

        std::vector<EventValue> getValues(void) const {
            std::vector<EventValue> values;
            for (size_t i = 0; i < getNumberOfValues(); ++i) {
                values.push_back(getValue(i));
            }
            return values;
        }

        std::string convertValueToString(const EventValue& value, bool hex) const;
    };


    /**
     *  Class holding data from the speedwire inverter reply packet, enriched by measurement type information and the interpreted measurement value.
     */
    class SpeedwireData : public SpeedwireRawData, public Measurement {
    public:
        std::string name;

        SpeedwireData(const Command command, const uint32_t id, const uint8_t conn, const SpeedwireDataType type, const time_t time,
            const void* data, const size_t data_size, const MeasurementType& mType, const Wire wire, const std::string &name);
        SpeedwireData(void);

        bool consume(const SpeedwireRawData& data);

        std::string toString(void) const;

        static std::vector<SpeedwireData> getAllPredefined(void);

        // pre-defined static instances
        static const SpeedwireData InverterDiscovery;              //!< Device discovery
        static const SpeedwireData InverterDeviceName;             //!< Device name string
        static const SpeedwireData InverterDeviceClass;            //!< Device class
        static const SpeedwireData InverterDeviceType;             //!< Device type
        static const SpeedwireData InverterSoftwareVersion;        //!< Software version
        static const SpeedwireData InverterPowerMPP1;              //!< Power on direct current inverter input MPP1
        static const SpeedwireData InverterPowerMPP2;              //!< Power on direct current inverter input MPP2
        static const SpeedwireData InverterVoltageMPP1;            //!< Voltage on direct current inverter input MPP1
        static const SpeedwireData InverterVoltageMPP2;            //!< Voltage on direct current inverter input MPP2
        static const SpeedwireData InverterCurrentMPP1;            //!< Current on direct current inverter input MPP1
        static const SpeedwireData InverterCurrentMPP2;            //!< Current on direct current inverter input MPP1
        static const SpeedwireData InverterPowerL1;                //!< Power on alternating current inverter output phase L1
        static const SpeedwireData InverterPowerL2;                //!< Power on alternating current inverter output phase L2
        static const SpeedwireData InverterPowerL3;                //!< Power on alternating current inverter output phase L3
        static const SpeedwireData InverterVoltageL1;              //!< Voltage on alternating current inverter output measured between L1 and N
        static const SpeedwireData InverterVoltageL2;              //!< Voltage on alternating current inverter output measured between L2 and N
        static const SpeedwireData InverterVoltageL3;              //!< Voltage on alternating current inverter output measured between L3 and N
        static const SpeedwireData InverterVoltageL1toL2;          //!< Voltage on alternating current inverter output measured between L1 and L2
        static const SpeedwireData InverterVoltageL2toL3;          //!< Voltage on alternating current inverter output measured between L2 and L3
        static const SpeedwireData InverterVoltageL3toL1;          //!< Voltage on alternating current inverter output measured between L3 and L1
        static const SpeedwireData InverterPowerFactor;            //!< Power factor on alternating current inverter output
        static const SpeedwireData InverterCurrentL1;              //!< Current on alternating current inverter output phase L1
        static const SpeedwireData InverterCurrentL2;              //!< Current on alternating current inverter output phase L2
        static const SpeedwireData InverterCurrentL3;              //!< Current on alternating current inverter output phase L3
        static const SpeedwireData InverterFrequency;              //!< Grid frequency on alternating current inverter output
        static const SpeedwireData InverterPowerACTotal;           //!< Total active power on alternating current inverter output
        static const SpeedwireData InverterReactivePowerTotal;     //!< Total reactive power on alternating current inverter output
        static const SpeedwireData InverterNominalPower;           //!< Total nominal power of the inverter device
        static const SpeedwireData InverterEnergyTotal;            //!< Total energy produced
        static const SpeedwireData InverterEnergyDaily;            //!< Daily energy produced
        static const SpeedwireData InverterGridExportEnergyTotal;  //!< Total energy fed into the grid - as reported by the emeter at the grid connection point
        static const SpeedwireData InverterGridImportEnergyTotal;  //!< Total energy consumed from the grid - as reported by the emeter at the grid connection point
        static const SpeedwireData InverterOperationTime;          //!< Total operation time of the inverter
        static const SpeedwireData InverterFeedInTime;             //!< Total feed-in time of the inverter
        static const SpeedwireData InverterOperationStatus;        //!< Inverter operation status
        static const SpeedwireData InverterUpdateStatus;           //!< Inverter update status
        static const SpeedwireData InverterMessageStatus;          //!< Inverter message status
        static const SpeedwireData InverterActionStatus;           //!< Inverter action status
        static const SpeedwireData InverterDescriptionStatus;      //!< Inverter description status
        static const SpeedwireData InverterErrorStatus;            //!< Inverter error status
        static const SpeedwireData InverterRelay;                  //!< Grid relay status

        static const SpeedwireData BatterySoftwareVersion;         //!< Software version
        static const SpeedwireData BatteryStateOfCharge;           //!< Battery charge status
        static const SpeedwireData BatteryDiagChargeCycles;        //!< Battery charge cycles
        static const SpeedwireData BatteryDiagTotalAhIn;           //!< Battery total Ah charged
        static const SpeedwireData BatteryDiagTotalAhOut;          //!< Battery total Ah discharged
        static const SpeedwireData BatteryTemperature;             //!< Battery temperature
        static const SpeedwireData BatteryVoltage;                 //!< Battery DC voltage
        static const SpeedwireData BatteryCurrent;                 //!< Battery DC current
        static const SpeedwireData BatteryPowerACTotal;            //!< Total power on alternating current battery inverter connector
        static const SpeedwireData BatteryPowerL1;                 //!< Power on alternating current battery inverter connector phase L1
        static const SpeedwireData BatteryPowerL2;                 //!< Power on alternating current battery inverter connector phase L2
        static const SpeedwireData BatteryPowerL3;                 //!< Power on alternating current battery inverter connector phase L3
        static const SpeedwireData BatteryVoltageL1;               //!< Voltage on alternating current battery inverter connector measured between L1 and N
        static const SpeedwireData BatteryVoltageL2;               //!< Voltage on alternating current battery inverter connector measured between L2 and N
        static const SpeedwireData BatteryVoltageL3;               //!< Voltage on alternating current battery inverter connector measured between L3 and N
        static const SpeedwireData BatteryVoltageL1toL2;           //!< Voltage on alternating current battery inverter connector measured between L1 and L2
        static const SpeedwireData BatteryVoltageL2toL3;           //!< Voltage on alternating current battery inverter connector measured between L2 and L3
        static const SpeedwireData BatteryVoltageL3toL1;           //!< Voltage on alternating current battery inverter connector measured between L3 and L1
        static const SpeedwireData BatteryCurrentL1;               //!< Current on alternating current battery inverter connector phase L1
        static const SpeedwireData BatteryCurrentL2;               //!< Current on alternating current battery inverter connector phase L2
        static const SpeedwireData BatteryCurrentL3;               //!< Current on alternating current battery inverter connector phase L3
        static const SpeedwireData BatteryGridVoltageL1;           //!< Voltage as reported by emeter on grid connection point measured between L1 and N
        static const SpeedwireData BatteryGridVoltageL2;           //!< Voltage as reported by emeter on grid connection point measured between L2 and N
        static const SpeedwireData BatteryGridVoltageL3;           //!< Voltage as reported by emeter on grid connection point measured between L3 and N
        static const SpeedwireData BatteryGridPositivePowerL1;     //!< Active power consumed from the grid as reported by emeter phase L1
        static const SpeedwireData BatteryGridPositivePowerL2;     //!< Active power consumed from the grid as reported by emeter phase L2
        static const SpeedwireData BatteryGridPositivePowerL3;     //!< Active power consumed from the grid as reported by emeter phase L3
        static const SpeedwireData BatteryGridNegativePowerL1;     //!< Active power fed to the grid as reported by emeter phase L1
        static const SpeedwireData BatteryGridNegativePowerL2;     //!< Active power fed to the grid as reported by emeter phase L2
        static const SpeedwireData BatteryGridNegativePowerL3;     //!< Active power fed to the grid as reported by emeter phase L3
        static const SpeedwireData BatteryGridReactivePowerL1;     //!< Reactive power as reported by emeter on grid connection point phase L1
        static const SpeedwireData BatteryGridReactivePowerL2;     //!< Reactive power as reported by emeter on grid connection point phase L2
        static const SpeedwireData BatteryGridReactivePowerL3;     //!< Reactive power as reported by emeter on grid connection point phase L3
        static const SpeedwireData BatteryGridReactivePower;       //!< Reactive power as reported by emeter on grid connection point total
        static const SpeedwireData BatterySetVoltage;              //!< Battery DC target set voltage
        static const SpeedwireData BatteryOperationStatus;         //!< Inverter operation status
        static const SpeedwireData BatteryRelay;                   //!< Grid relay status
        static const SpeedwireData BatteryType;                    //!< Battery type

        static const SpeedwireData InverterPowerDCTotal;           //!< Total power on direct current inverter inputs MPP1 + MPP2
        static const SpeedwireData InverterPowerLoss;              //!< Total power loss
        static const SpeedwireData InverterPowerEfficiency;        //!< Total power efficiency

        static const SpeedwireData HouseholdPowerTotal;            //!< Total power consumption of the household
        static const SpeedwireData HouseholdIncomeTotal;           //!< Income generated
        static const SpeedwireData HouseholdIncomeFeedIn;          //!< Income generated from grid feed-in
        static const SpeedwireData HouseholdIncomeSelfConsumption; //!< Income generated from self-consumption

        static const SpeedwireData YieldByMinute;                  //!< Energy yield in 5 minute intervals
        static const SpeedwireData YieldByDay;                     //!< Energy yield in 24 hour intervals
        static const SpeedwireData Event;                          //!< Device event
    };


    /**
     *  Class implementing a query map for speedwire inverter reply data.
     *  The class extends std::map<uint32_t, SpeedwireData> and adds functionality to deal with keys derived from
        a SpeedwireRawData instance.
     */
    class SpeedwireDataMap : public std::map<uint32_t, SpeedwireData> {
        static SpeedwireDataMap globalMap;  // map containing all known definitions

    public:
        /**
         *  Default constructor.
         */
        SpeedwireDataMap(void) : std::map<uint32_t, SpeedwireData>() {}

        /**
         *  Construct a new map from the given vector of SpeedwireData elements.
         *  @param elements the vector of SpeedwireData elements
         */
        SpeedwireDataMap(const std::vector<SpeedwireData>& elements) {
            this->add(elements);
        }

        /**
         *  Add a new element to the map of speedwire inverter reply data elements.
         *  @param element The SpeedwireData element to be added to the map
         */
        void add(const SpeedwireData& element) { operator[](element.toKey()) = element; }

        /**
         *  Add a vector of elements to the map of emeter obis data elements.
         *  @param elements The vector of ObisData element to be added to the map
         */
        void add(const std::vector<SpeedwireData>& elements) {
            for (auto& e : elements) {
                add(e);
            }
        }

        /**
         *  Remove the given element from the map of emeter obis data elements.
         *  @param element The ObisData element to be removed from the map
         */
        void remove(const SpeedwireData& entry) {
            erase(entry.toKey());
        }

        static SpeedwireDataMap &getGlobalMap(void);
    };

}   // namespace libspeedwire

#endif
