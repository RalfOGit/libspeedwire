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
     * @param element A reference to an ObisData instance, holding output data of the ObisFilter.
     */
    virtual void consume(const uint32_t serial, ObisData& element) = 0;
};


/**
 *  Interface to be implemented by the consumer of speedwire inverter reply data.
 */
class SpeedwireConsumer {
public:
    /** Virtual destructor */
    virtual ~SpeedwireConsumer(void) {}

    /**
     *  Consume a speedwire reply data element
     *  @param element The reply data element
     */
    virtual void consume(const uint32_t serial, SpeedwireData& element) = 0;
};

#endif
