#ifndef __LIBSPEEDWIRE_SPEEDWIREDATA_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDATA_HPP__

#include <cstdint>
#include <string>
#include <stdio.h>
#include <map>
#include <Measurement.hpp>
#include <MeasurementType.hpp>
#include <MeasurementValues.hpp>
#include <SpeedwireByteEncoding.hpp>

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
        Signed32   = 0x40
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
         *  @return The key for this instance
         */
        uint32_t toKey(void) const { return id | conn; }

        std::string toHexString(void) const;
        std::string toString(void) const;

        size_t getNumberOfValues(void) const;
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
        double convertValueToDouble(uint32_t value) const { return (double)value; }

        std::string convertValueToString(uint32_t value) const {
            if (value == nan) { return "NaN"; }
            if (value == eod) { return "EoD"; }
            char byte[32];
            snprintf(byte, sizeof(byte), "0x%08lx", value);
            return std::string(byte);
        }
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
        double convertValueToDouble(int32_t value) const { return (double)value; }

        std::string convertValueToString(int32_t value) const {
            if (value == nan) { return "NaN"; }
            char byte[32];
            snprintf(byte, sizeof(byte), "0x%08lx", value);
            return std::string(byte);
        }
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

        SpeedwireRawDataStatus32(const SpeedwireRawData &raw_data) : base(raw_data) {}

        size_t getNumberOfValues(void) const { return base.data_size / value_size; }
        bool isNanValue(uint32_t value) const { return ((value & value_mask) == nan); }
        bool isEoDValue(uint32_t value) const { return ((value & value_mask) == eod); }
        uint32_t getValue(size_t pos) const { return SpeedwireByteEncoding::getUint32LittleEndian(base.data + pos * value_size); }

        /** Get the index of the data value marked with 0x01000000; this is the selected value in the list of values */
        size_t getSelectionIndex(void) const {
            size_t num_values = getNumberOfValues();
            for (size_t i = 0; i < num_values; ++i) {
                uint32_t svalue = getValue(i);
                if ((svalue & marker_mask) == 0x01000000) {
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
            if (value == (0x01000000 | nan)) { return "Sel.NaN"; }
            if (value == eod) { return "EoD"; }
            char byte[32];
            snprintf(byte, sizeof(byte), "0x%08lx", value);
            return std::string(byte);
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
        std::string getValue(size_t pos) { return std::string((char*)base.data + pos * value_size, base.data_size - pos * value_size); }

    };


    /**
     *  Class holding data from the speedwire inverter reply packet, enriched by measurement type information and the interpreted measurement value.
     */
    class SpeedwireData : public SpeedwireRawData, public Measurement {
    public:
        std::string name;

        SpeedwireData(const uint32_t command, const uint32_t id, const uint8_t conn, const SpeedwireDataType type, const time_t time,
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
        static const SpeedwireData InverterVoltageL1;              //!< Voltage on alternating current inverter output phase L1, measured between L1 and N
        static const SpeedwireData InverterVoltageL2;              //!< Voltage on alternating current inverter output phase L2, measured between L2 and N
        static const SpeedwireData InverterVoltageL3;              //!< Voltage on alternating current inverter output phase L3, measured between L3 and N
        static const SpeedwireData InverterVoltageL1toL2;          //!< Voltage on alternating current inverter output phase L1, measured between L1 and L2
        static const SpeedwireData InverterVoltageL2toL3;          //!< Voltage on alternating current inverter output phase L1, measured between L2 and L3
        static const SpeedwireData InverterVoltageL3toL1;          //!< Voltage on alternating current inverter output phase L1, measured between L3 and L1
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
        static const SpeedwireData InverterStatus;                 //!< Inverter operation status
        static const SpeedwireData InverterRelay;                  //!< Grid relay status

        static const SpeedwireData InverterPowerDCTotal;           //!< Total power on direct current inverter inputs MPP1 + MPP2
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
        static SpeedwireDataMap allPredefined;

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

        static const SpeedwireDataMap &getAllPredefined(void);
    };

}   // namespace libspeedwire

#endif
