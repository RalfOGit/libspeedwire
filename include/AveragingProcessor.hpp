#ifndef __AVERAGINGPROCESSOR_HPP__
#define __AVERAGINGPROCESSOR_HPP__

#include <cstdint>
#include <Consumer.hpp>
#include <ObisData.hpp>
#include <ObisFilter.hpp>
#include <SpeedwireData.hpp>
#include <Measurement.hpp>
#include <Producer.hpp>


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

    unsigned long averagingTime;                //!< Averaging time constant.
    std::vector<AveragingState> states;         //!< Array holding averaging states for alle knowne speedwire devices
    Producer& producer;                         //!< Reference to the producer receiving averaged data.

    int initializeState(const uint32_t serial_number, const DeviceType& device_type);
    int findStateIndex (const uint32_t serial_number);
    void consume(const uint32_t serial_number, const DeviceType& device_type, MeasurementType& type, MeasurementValue& value, Wire& wire);

public:

    AveragingProcessor(const unsigned long averagingTime, Producer& producer);
    ~AveragingProcessor(void);

    virtual void consume(const uint32_t serial_number, ObisData &element);
    virtual void consume(const uint32_t serial_number, SpeedwireData& element);
};

#endif
