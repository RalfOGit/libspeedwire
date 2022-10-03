#ifndef __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__
#define __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <SpeedwireTime.hpp>

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
            write_pointer = 0;
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
            const TimestampDoublePair pair(value, time);
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
         *  Get the index in the ring buffer time-wise closest to the given time.
         *  @return index in ring buffer
         */
        const size_t findClosestIndex(const uint32_t time) const {
            if (values.size() > 0) {
                uint64_t min_diff = (uint64_t)-1;
                size_t min_index = 0;
                for (size_t i = 0; i < values.size(); ++i) {
                    uint64_t diff = SpeedwireTime::calculateAbsTimeDifference(time, values[i].time);
                    if (diff < min_diff) {
                        min_diff = diff;
                        min_index = i;
                    }
                }
                return min_index;
            }
            return (size_t)-1;
        }

        /**
         *  Get a reference to the measurement in the ring buffer time-wise closest to the given time.
         *  @param the time to compare with
         *  @return reference to TimestampDoublePair
         */
        const TimestampDoublePair& findClosestMeasurement(const uint32_t time) const {
            const size_t closest_index = findClosestIndex(time);
            if (closest_index != (size_t)-1) {
                return values[closest_index];
            }
            return TimestampDoublePair::defaultPair;    // this is to avoid an exception
        }

        /**
         *  Interpolate the two measurement values time-wise closest to the given time.
         *  @param the time to compare with
         *  @return the interpolated measurement value
         */
        const double interpolateClosestValues(const uint32_t time) const {
            const size_t index_center = findClosestIndex(time);
            if (index_center != (size_t)-1) {
                const size_t num_measurements = getNumberOfMeasurements();
                if (num_measurements > 1) {
                    const size_t index_before = (index_center > 0 ? (index_center - 1) : (num_measurements - 1));
                    const size_t index_after  = (index_center < (num_measurements - 1) ? (index_center + 1) : 0);
                    const uint64_t diff_before = SpeedwireTime::calculateAbsTimeDifference(time, values[index_before].time);
                    const uint64_t diff_center = SpeedwireTime::calculateAbsTimeDifference(time, values[index_center].time);
                    const uint64_t diff_after  = SpeedwireTime::calculateAbsTimeDifference(time, values[index_after].time);
                    if (diff_before <= diff_after) {
                        return (diff_center * values[index_before].value + diff_before * values[index_center].value) / (diff_before + diff_center);
                    }
                    return (diff_after * values[index_center].value + diff_center * values[index_after].value) / (diff_center + diff_after);
                }
                return values[index_center].value;
            }
            return 0.0;
        }

        /**
         *  Calculate the average value of all measurements in the ring buffer.
         *  @return average value
         */
        double calculateAverageValue(void) const {
            double sum = 0.0;
            for (const auto& m : values) {
                sum += m.value;
            }
            return sum / values.size();
        }
    };

}   // namespace libspeedwire

#endif
