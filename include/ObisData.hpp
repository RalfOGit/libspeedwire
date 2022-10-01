#ifndef __LIBSPEEDWIRE_OBISDATA_HPP__
#define __LIBSPEEDWIRE_OBISDATA_HPP__

#include <cstdint>
#include <stdio.h>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <Measurement.hpp>
#include <MeasurementType.hpp>
#include <MeasurementValues.hpp>
#include <SpeedwireEmeterProtocol.hpp>

namespace libspeedwire {

    /**
     *  Class encapsulating an obis data definition used inside speedwire emeter packets.
     */
    class ObisType {
    public:
        uint8_t channel;    //!< obis measurement channel
        uint8_t index;      //!< obis measurement index; i.e. measurement quantity
        uint8_t type;       //!< obis measurement type
        uint8_t tariff;     //!< obis tariff

        ObisType(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff);

        bool equals(const ObisType& other) const;

        std::string toString(void) const;
        std::string toString(const uint32_t value) const;
        std::string toString(const uint64_t value) const;

        std::array<uint8_t, 12> toByteArray(void) const;

        uint32_t toKey(void) const;
    };


    /**
     *  Class holding an emeter measurement together with its corresponding ObisType definition and its MeasurementType definition.
     */
    class ObisData : public ObisType, public Measurement {
    public:

        ObisData(void);
        ObisData(const uint8_t channel, const uint8_t index, const uint8_t type, const uint8_t tariff, const MeasurementType& measurementType, const Wire& line);

        bool equals(const ObisType& other) const;

        void print(FILE* file) const;

        std::array<uint8_t, 12> toByteArray(void) const;

        static std::vector<ObisData> getAllPredefined(void);

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
        static const ObisData PositiveReactivePowerTotal;
        static const ObisData PositiveReactivePowerL1;
        static const ObisData PositiveReactivePowerL2;
        static const ObisData PositiveReactivePowerL3;
        static const ObisData PositiveReactiveEnergyTotal;
        static const ObisData PositiveReactiveEnergyL1;
        static const ObisData PositiveReactiveEnergyL2;
        static const ObisData PositiveReactiveEnergyL3;
        static const ObisData NegativeReactivePowerTotal;
        static const ObisData NegativeReactivePowerL1;
        static const ObisData NegativeReactivePowerL2;
        static const ObisData NegativeReactivePowerL3;
        static const ObisData NegativeReactiveEnergyTotal;
        static const ObisData NegativeReactiveEnergyL1;
        static const ObisData NegativeReactiveEnergyL2;
        static const ObisData NegativeReactiveEnergyL3;
        static const ObisData PositiveApparentPowerTotal;
        static const ObisData PositiveApparentPowerL1;
        static const ObisData PositiveApparentPowerL2;
        static const ObisData PositiveApparentPowerL3;
        static const ObisData PositiveApparentEnergyTotal;
        static const ObisData PositiveApparentEnergyL1;
        static const ObisData PositiveApparentEnergyL2;
        static const ObisData PositiveApparentEnergyL3;
        static const ObisData NegativeApparentPowerTotal;
        static const ObisData NegativeApparentPowerL1;
        static const ObisData NegativeApparentPowerL2;
        static const ObisData NegativeApparentPowerL3;
        static const ObisData NegativeApparentEnergyTotal;
        static const ObisData NegativeApparentEnergyL1;
        static const ObisData NegativeApparentEnergyL2;
        static const ObisData NegativeApparentEnergyL3;
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
        static const ObisData SoftwareVersion;
        static const ObisData EndOfData;
        static const ObisData SignedActivePowerTotal;
        static const ObisData SignedActivePowerL1;
        static const ObisData SignedActivePowerL2;
        static const ObisData SignedActivePowerL3;
    };


    /**
     *  Class implementing a map for emeter obis data.
     *  The class extends std::map<uint32_t, ObisData>.
     */
    class ObisDataMap : public std::map<uint32_t, ObisData> {
    public:
        //! Serial number of the device that provided this data.
        uint32_t serial_number;

        /**
         *  Add a new element to the map of semeter obis data.elements.
         *  @param element The ObisData element added to the map
         */
        void add(const ObisData& element) { operator[](element.toKey()) = element; }
    };

}   // namespace libspeedwire

#endif
