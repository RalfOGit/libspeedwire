#ifndef __PRODUCER_HPP__
#define __PRODUCER_HPP__

#include <Measurement.hpp>


/**
 *  Interface to be implemented by any producer like InfluxDBProducer.
 */
class Producer {
public:
    virtual void flush(void) = 0;
    virtual void produce(const std::string &device, const MeasurementType &type, const Wire, const double value) = 0;
};

#endif
