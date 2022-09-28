#ifndef __LIBSPEEDWIRE_MEASUREMENT_HPP__
#define __LIBSPEEDWIRE_MEASUREMENT_HPP__

#include <cstdint>
#include <string>

namespace libspeedwire {

    /**
     *  Class encapsulating the value and other variable properties of a measurement.
     */
    class MeasurementValue {
    public:
        double       value;         //!< Value of the current measurement, which can be the most recent measurement or an average value
        std::string  value_string;  //!< Value of the current measurement, if it is not a numeric value (e.g. software version, etc)
        uint32_t     timer;         //!< The current, i.e. most recent timestamp
        uint32_t     elapsed;       //!< Time elapsed from previous timestamp to current timestamp
        double       sumValue;      //!< The sum of previous and current measurements
        unsigned int counter;       //!< The number of measurements included in sumValue
        double       lastValue;     //!< Value of the most recent measurement
        bool         initial;       //!< Flag indicating that this instance is new

        MeasurementValue(void);
        void setValue(int32_t  raw_value, unsigned long divisor);
        void setValue(uint32_t raw_value, unsigned long divisor);
        void setValue(uint64_t raw_value, unsigned long divisor);
        void setValue(const std::string& raw_value);
        void setTimer(uint32_t timer);
    };

}   // namespace libspeedwire

#endif
