#ifndef __LIBSPEEDWIRE_PRODUCER_HPP__
#define __LIBSPEEDWIRE_PRODUCER_HPP__

#include <Measurement.hpp>

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
         * @param serial_number The serial number of the device generating the data
         * @param type The measurement type of the given data
         * @param wire The Wire enumeration value
         * @param value The data value itself
         */
        virtual void produce(const uint32_t serial_number, const MeasurementType& type, const Wire wire, const double value) = 0;
    };

}   // namespace libspeedwire

#endif
