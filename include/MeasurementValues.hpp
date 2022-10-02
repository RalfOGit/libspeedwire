#ifndef __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__
#define __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <LocalHost.hpp>

namespace libspeedwire {

    /**
     *  Class encapsulating a value timestamp pair, where the value is a double value.
     */
    class TimestampDoublePair {
    public:
        double      value;      //!< Measurement value
        uint32_t    time;       //!< Measurement time

        TimestampDoublePair(void) : value(0.0), time(0) {}
        TimestampDoublePair(const double v, const uint32_t t) : value(v), time(t) {}

        static TimestampDoublePair defaultPair;
    };


    /**
     *  Class encapsulating a ring buffer of measurement values together with their timesamps.
     */
    class MeasurementValues {
    public:
        std::vector<TimestampDoublePair>  values;   //!< Array of measurements
        std::string value_string;                   //!< String value, e.g. to hold the firmware version or similar
        size_t write_pointer;                       //!< Write pointer pointing to the next element to write to

        /**
         * Constructor.
         * @param capacity Maximum number of measurements
         */
        MeasurementValues(const int capacity) :
            write_pointer(0),
            values(capacity) {
        }

        /**
         *  Delete all measurements from the ring buffer.
         */
        void clear(void) {
            values.clear();
        }

        /**
         *  Get maximum number of measurements that can be stored in the ring buffer.
         *  @return the maximum number
         */
        size_t getMaximumNumberOfMeasurements(void) const {
            return values.capacity();
        }

        /**
         *  Set maximum number of measurements that can be stored in the ring buffer.
         *  This will clear any measurements before resizing the ring buffer.
         *  @param new_capacity the maximum number
         */
        void setMaximumNumberOfMeasurements(const size_t new_capacity) {
            values.clear();
            values.reserve(new_capacity);
        }
        
        void addMeasurement(const int32_t  raw_value, const unsigned long divisor, const uint32_t time) {
            addMeasurement((double)raw_value / (double)divisor, time);
        }
        void addMeasurement(const uint32_t raw_value, const unsigned long divisor, const uint32_t time) {
            addMeasurement((double)raw_value / (double)divisor, time);
        }
        void addMeasurement(const uint64_t raw_value, const unsigned long divisor, const uint32_t time) {
            addMeasurement((double)raw_value / (double)divisor, time);
        }

        /**
         *  Add a new measurement to the ring buffer. If the buffer is full, the oldest measurement is replaced.
         *  @param value the measurement value
         *  @param time the measurement time
         */
        void addMeasurement(const double value, const uint32_t time) {
            TimestampDoublePair pair(value, time);
            if (values.size() > write_pointer) {
                values[write_pointer] = pair;
            }
            else {
                values.push_back(pair);
            }
            if (++write_pointer >= values.capacity()) {
                write_pointer = 0;
            }
        }

        /**
         *  Get number of measurements that are currently stored in the ring buffer.
         *  @return the number 
         */
        size_t getNumberOfMeasurements(void) const {
            return values.size();
        }

        /**
         *  Get a reference to the most recent measurement in the ring buffer.
         *  @return reference to TimestampDoublePair
         */
        const TimestampDoublePair &getMostRecentMeasurement(void) const {
            if (write_pointer > 0) {
                return values[write_pointer - 1];
            }
            if (values.size() > 0) {
                return values[values.size() - 1];
            }
            return TimestampDoublePair::defaultPair;    // this is to avoid an exception
        }

        /**
         *  Get a reference to the measurement in the ring buffer with the measurement time closest to the given time.
         *  @param the time to compare with
         *  @return reference to TimestampDoublePair
         */
        const TimestampDoublePair& findClosestMeasurement(const uint32_t time) const {
            if (values.size() > 0) {
                uint64_t min_diff = LocalHost::calculateAbsTimeDifference(time, values[0].time);
                size_t min_index = 0;
                for (size_t i = 1; i < values.size(); ++i) {
                    uint64_t diff = LocalHost::calculateAbsTimeDifference(time, values[i].time);
                    if (diff < min_diff) {
                        min_diff = diff;
                        min_index = i;
                    }
                }
                return values[min_index];
            }
            return TimestampDoublePair::defaultPair;    // this is to avoid an exception
        }

        /**
         *  Calculate the average value of all measurements in the ring buffer.
         *  @return average value
         */
        double calculateAverageValue(void) const {
            double sum = 0.0;
            for (size_t i = 0; i < values.size(); ++i) {
                sum += values[i].value;
            }
            return sum / values.size();
        }
    };

}   // namespace libspeedwire

#endif
