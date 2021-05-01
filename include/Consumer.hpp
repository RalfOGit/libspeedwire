#ifndef __CONSUMER_HPP__
#define __CONSUMER_HPP__

#include <ObisData.hpp>
#include <SpeedwireData.hpp>


/**
 *  Interface to be implemented by any obis consumer; that is any consumer that likes to receive data from class ObisFilter.
 */
class ObisConsumer {
public:
    /** Virtual destructor. */
    virtual ~ObisConsumer(void) {}

    /**
     * Callback to produce the given obis data to the next stage in the processing pipeline.
     * @param serial_number The serial number of the originating emeter device.
     * @param element A reference to a received ObisData instance, holding output data of the ObisFilter.
     */
    virtual void consume(const uint32_t serial_number, ObisData& element) = 0;

    /**
     * Callboack to notify that the last obis data in the emeter packet has been processed.
     * @param serial_number The serial number of the originating emeter device.
     * @param timestamp The timestamp associated with the just finished emeter packet.
     */
    virtual void endOfObisData(const uint32_t serial_number, const uint32_t timestamp) {}
};


/**
 *  Interface to be implemented by the consumer of speedwire inverter reply data.
 */
class SpeedwireConsumer {
public:
    /** Virtual destructor */
    virtual ~SpeedwireConsumer(void) {}

    /**
     * Consume a speedwire reply data element
     * @param serial_number The serial number of the originating inverter device.
     * @param element A reference to a received SpeedwireData instance.
     */
    virtual void consume(const uint32_t serial_number, SpeedwireData& element) = 0;

    /**
     * Callboack to notify that the last data in the inverter packet has been processed.
     * @param serial_number The serial number of the originating inverter device.
     * @param timestamp The timestamp associated with the just finished inverter packet.
     */
    virtual void endOfSpeedwireData(const uint32_t serial_number, const uint32_t timestamp) {}
};

#endif
