#ifndef __LIBSPEEDWIRE_PRODUCER_HPP__
#define __LIBSPEEDWIRE_PRODUCER_HPP__

#include <MeasurementType.hpp>
#include <SpeedwireDevice.hpp>

namespace libspeedwire {

    /**
     *  Interface to be implemented by any producer like InfluxDBProducer.
     */
    class Producer {
    public:
        /** Virtual destructor. */
        virtual ~Producer(void) {}

        /** Flush any internally cached data to the next stage in the processing pipeline. */
        virtual void flush(void) = 0;

        /**
         * Callback to produce the given data to the next stage in the processing pipeline.
         * @param device The device generating the data
         * @param type The measurement type of the given data
         * @param wire The Wire enumeration value
         * @param value The data value itself
         * @param time_in_ms The measurement timestamp in ms since unix epoch start
         */
        virtual void produce(const SpeedwireDevice& device, const MeasurementType& type, const Wire wire, const double value, const uint32_t time_in_ms = 0) = 0;
    };

}   // namespace libspeedwire

#endif
