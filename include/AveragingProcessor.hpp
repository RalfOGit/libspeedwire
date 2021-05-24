#ifndef __LIBSPEEDWIRE_AVERAGINGPROCESSOR_HPP__
#define __LIBSPEEDWIRE_AVERAGINGPROCESSOR_HPP__

#include <cstdint>
#include <Consumer.hpp>
#include <ObisData.hpp>
#include <ObisFilter.hpp>
#include <SpeedwireData.hpp>
#include <Measurement.hpp>
#include <Producer.hpp>

namespace libspeedwire {

    /**
     *  Class AveragingProcessor implements the temporal averaging processing of obis elements received from emeter
     *  packets and inverter reply packets; this is useful to reduce the amount of data fed to the InfluxDB producer.
     */
    class AveragingProcessor : public ObisConsumer, SpeedwireConsumer {

    protected:

        //! Device type.
        enum class DeviceType {
            EMETER,     //!< An emeter device.
            INVERTER    //!< An inverter device.
        };

        //! Struct holding a block of averaging related information for a given speedwire device.
        typedef struct {
            uint32_t      serialNumber;             //!< Serial number of the speedwire device.
            DeviceType    deviceType;               //!< Device type.
            unsigned long remainder;                //!< Remainding time for averaging emeter obis packets.
            uint32_t      currentTimestamp;         //!< Timestamp of the most recently received emeter obis packet.
            bool          currentTimestampIsValid;  //!< The current emeter timestamp has been initialized.
            bool          averagingTimeReached;     //!< Boolean indicating that the averaging time has been reached with this emeter obis packet.
        } AveragingState;

        unsigned long averagingTime;                            //!< Averaging time constant.
        std::vector<AveragingState> states;                     //!< Array holding averaging states for alle knowne speedwire devices
        std::vector<ObisConsumer*> obisConsumerTable;           //!< Table of registered ObisConsumer
        std::vector<SpeedwireConsumer*> speedwireConsumerTable; //!< Table of registered SpeedwireConsumer

        int initializeState(const uint32_t serial_number, const DeviceType& device_type);
        int findStateIndex(const uint32_t serial_number);
        bool process(const uint32_t serial_number, const DeviceType& device_type, MeasurementType& type, MeasurementValue& value);

    public:

        AveragingProcessor(const unsigned long averagingTime);
        ~AveragingProcessor(void);

        void addConsumer(ObisConsumer& obis_consumer);
        void addConsumer(SpeedwireConsumer& speedwire_consumer);

        virtual void consume(const uint32_t serial_number, ObisData& element);
        virtual void consume(const uint32_t serial_number, SpeedwireData& element);
        virtual void endOfObisData(const uint32_t serial_number, const uint32_t time);
        virtual void endOfSpeedwireData(const uint32_t serial_number, const uint32_t time);
    };

}   // namespace libspeedwire

#endif
