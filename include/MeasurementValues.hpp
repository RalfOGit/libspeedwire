#ifndef __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__
#define __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <RingBuffer.hpp>
#include <SpeedwireTime.hpp>

namespace libspeedwire {

    /**
     *  Class encapsulating a value timestamp pair, where the value is a double value.
     */
    class TimestampDoublePair {
    public:
        double      value;      //!< Measurement value
        uint32_t    time;       //!< Measurement time

        TimestampDoublePair(void) : value(999999.0), time(0) {}  // choose an unlikely value to help with debugging
        TimestampDoublePair(const double v, const uint32_t t) : value(v), time(t) {}

        static TimestampDoublePair defaultPair;
    };


    /**
     *  Class encapsulating a ring buffer of measurement values together with their timesamps.
     */
    class MeasurementValues : public RingBuffer<TimestampDoublePair> {
    public:
        std::string value_string;                   //!< String value, e.g. to hold the firmware version or similar

        /**
         * Constructor.
         * @param capacity Maximum number of measurements
         */
        MeasurementValues(const int capacity) : RingBuffer(capacity) {}

        /**
         *  Add a new measurement to the ring buffer. If the buffer is full, the oldest measurement is replaced.
         *  @param value the measurement value
         *  @param time the measurement time
         */
        void addMeasurement(const double value, const uint32_t time) {
            const TimestampDoublePair pair(value, time);
            addNewElement(pair);
        }

        /**
         *  Get the index in the ring buffer time-wise closest to the given time.
         *  @return index in ring buffer
         */
        const size_t findClosestIndex(const uint32_t time) const {
            if (getNumberOfElements() > 0) {
                uint64_t min_diff = (uint64_t)-1;
                size_t min_index = 0;
                for (size_t i = 0; i < getNumberOfElements(); ++i) {
                    uint64_t diff = SpeedwireTime::calculateAbsTimeDifference(time, at(i).time);
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
                return at(closest_index);
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
                const size_t num_measurements = getNumberOfElements();
                if (num_measurements > 1) {
                    const size_t index_before = (index_center > 0 ? (index_center - 1) : index_center);
                    const size_t index_after  = (index_center < (num_measurements - 1) ? (index_center + 1) : index_center);
                    const uint64_t diff_before = SpeedwireTime::calculateAbsTimeDifference(time, at(index_before).time);
                    const uint64_t diff_center = SpeedwireTime::calculateAbsTimeDifference(time, at(index_center).time);
                    const uint64_t diff_after  = SpeedwireTime::calculateAbsTimeDifference(time, at(index_after).time);
                    if (index_after == index_center || (index_before != index_center && diff_before <= diff_after)) {
                        // the element prior to the closest element is time-wise closer than the element after the closest element,
                        // or, the closest index is the newest element; in both cases interpolate with the element prior to newest
                        return (diff_center * at(index_before).value + diff_before * at(index_center).value) / (diff_before + diff_center);
                    }
                    // the element after the closest element is time-wise closer than the element prior the closest element,
                    // or, the closest index is the oldest element; in both cases interpolate with the prior to oldest element
                    return (diff_after * at(index_center).value + diff_center * at(index_after).value) / (diff_center + diff_after);
                }
                return at(index_center).value;
            }
            return 0.0;
        }

        /**
         *  Calculate the average value of all measurements in the ring buffer.
         *  @return average value
         */
        double calculateAverageValue(void) const {
            double sum = 0.0;
            for (const auto& m : data_vector) {
                sum += m.value;
            }
            return sum / data_vector.size();
        }
    };

}   // namespace libspeedwire

#endif
