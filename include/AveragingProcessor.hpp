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
            unsigned long averagingTime;            //!< Averaging time for data packets.
            unsigned long remainder;                //!< Remainding time for averaging data packets.
            uint32_t      currentTimestamp;         //!< Timestamp of the most recently received data packet.
            bool          currentTimestampIsValid;  //!< The current emeter timestamp has been initialized.
            bool          averagingTimeReached;     //!< Boolean indicating that the averaging time has been reached with this emeter obis packet.
        } AveragingState;

        unsigned long averagingTimeObisData;                    //!< Averaging time constant for obis data.
        unsigned long averagingTimeSpeedwireData;               //!< Averaging time constant for speedwire data.
        std::vector<AveragingState> states;                     //!< Array holding averaging states for alle knowne speedwire devices
        std::vector<ObisConsumer*> obisConsumerTable;           //!< Table of registered ObisConsumer
        std::vector<SpeedwireConsumer*> speedwireConsumerTable; //!< Table of registered SpeedwireConsumer

        int initializeState(const uint32_t serial_number, const DeviceType& device_type);
        int findStateIndex(const uint32_t serial_number);
        bool process(const SpeedwireDevice& device, const DeviceType& device_type, Measurement& measurement);

    public:

        AveragingProcessor(const unsigned long averagingTimeObisData, const unsigned long averagingTimeSpeedwireData);
        ~AveragingProcessor(void);

        void addConsumer(ObisConsumer& obis_consumer);
        void addConsumer(SpeedwireConsumer& speedwire_consumer);

        virtual void consume(const SpeedwireDevice& device, ObisData& element);
        virtual void consume(const SpeedwireDevice& device, SpeedwireData& element);
        virtual void endOfObisData(const SpeedwireDevice& device, const uint32_t time);
        virtual void endOfSpeedwireData(const SpeedwireDevice& device, const uint32_t time);
    };

}   // namespace libspeedwire

#endif
