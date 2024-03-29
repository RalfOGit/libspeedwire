#ifndef __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__
#define __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <float.h>
#include <RingBuffer.hpp>
#include <SpeedwireTime.hpp>

namespace libspeedwire {

    /**
     *  Class encapsulating a value-timestamp pair, where the value is a double value.
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
     *  It is assumed that measurement values are added to the ring buffer with monotically increasing timestamps.
     */
    class MeasurementValues : public RingBuffer<TimestampDoublePair> {
    public:
        std::string value_string;                   //!< String value, e.g. to hold the firmware version or similar

        /**
         * Constructor.
         * @param capacity Maximum number of measurements
         */
        MeasurementValues(const size_t capacity) : RingBuffer(capacity) {}

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
        size_t findClosestIndex(const uint32_t time) const {
            if (getNumberOfElements() > 0) {
                // binary search
                size_t low = 0;
                size_t high = getNumberOfElements() - 1;
                while ((low + 1) < high) {
                    const size_t mid = (low + high) / 2u;
                    if (SpeedwireTime::calculateTimeDifference(time, at(mid).time) > 0) {  // use signed difference
                        low = mid;
                    }
                    else {
                        high = mid;
                    }
                }
                const bool low_is_closer = (SpeedwireTime::calculateAbsTimeDifference(time, at(low).time) < SpeedwireTime::calculateAbsTimeDifference(time, at(high).time));
                return (low_is_closer ? low : high);
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
        double interpolateClosestValues(const uint32_t time) const {
            const size_t index_center = findClosestIndex(time);
            if (index_center != (size_t)-1) {
                const size_t num_measurements = getNumberOfElements();
                if (num_measurements > 1) {
                    const size_t index_before = (index_center > 0 ? (index_center - 1) : index_center);
                    const size_t index_after  = (index_center < (num_measurements - 1) ? (index_center + 1) : index_center);
                    const uint32_t diff_before = SpeedwireTime::calculateAbsTimeDifference(time, at(index_before).time);
                    const uint32_t diff_center = SpeedwireTime::calculateAbsTimeDifference(time, at(index_center).time);
                    const uint32_t diff_after  = SpeedwireTime::calculateAbsTimeDifference(time, at(index_after).time);
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
         *  Estimate the sample mean, aka average value, of all measurements in the ring buffer.
         *  @return average value
         */
        double estimateMean(void) const {
            double sum = 0.0;
            for (const auto& m : data_vector) {
                sum += m.value;
            }
            return sum / data_vector.size();
        }

        /**
         *  Estimate the sample mean, aka average value, over the given subset of measurements in the ring buffer.
         *  @param from start index
         *  @param to end index; the measurement with index end is included
         *  @return average value
         */
        double estimateMean(const size_t from, const size_t to) const {
            double sum = 0.0;
            for (size_t index = from; index <= to; ++index) {
                sum += at(index).value;
            }
            return sum / (to - from + 1);
        }

        /**
         *  Estimate sample mean and sample variance values over the given subset of measurements in the ring buffer.
         *  @param from start index
         *  @param to end index; the measurement with index end is included
         *  @param the sample mean result
         *  @param the sample variance result
         */
        void estimateMeanAndVariance(const size_t start_index, const size_t end_index, double& mean, double& var) const {
            const size_t n_values = end_index - start_index + 1;

            double y_sum = 0.0, y_sq_sum = 0.0;
            for (size_t index = start_index; index <= end_index; ++index) {
                const double value = at(index).value;
                y_sum    += value;
                y_sq_sum += value * value;
            }
            mean = y_sum / n_values;
            // sample var = sum(y - mean) / (n_values - 1) is equivalent to (sum(y) / n_values - mean * mean) * (n_values / (n_values - 1))
            var  = (n_values <= 1 ? FLT_MAX : (y_sq_sum - mean * y_sum) / (n_values - 1));
        }

        /**
         *  Estimate linear regression over the given subset of measurements in the ring buffer.
         *  @param from start index
         *  @param to end index; the measurement with index end is included
         *  @param the sample mean result
         *  @param the sample variance result
         *  @param the slope result
         */
        void estimateLinearRegression(const size_t start_index, const size_t end_index, double& mean, double& var, double& slope) const {
            const size_t n_values_minus_1 = end_index - start_index;
            const size_t n_values         = n_values_minus_1 + 1;

            // estimate mean of y-coordinate, sample variance of y coordinate and also the xy covariance
            double y_sum = 0.0, y_sq_sum = 0.0, xy_sum = 0.0;
            for (size_t index = start_index; index <= end_index; ++index) {
                const double value = at(index).value;
                y_sum    += value;
                y_sq_sum += value * value;
                xy_sum   += value * (index - start_index);
            }
            mean = y_sum / n_values;
            var  = (n_values <= 1 ? FLT_MAX : (y_sq_sum - mean * y_sum) / n_values_minus_1);

            // calculate mean and variance of x coordinate
            // calculate x_var from sum of squared ints: 1^2 + 2^2 + 3^2 + ... + n^2 = [n(n+1)(2n+1)] / 6
#if 0
            // more readable but less accurate code
            const double x_mean = n_values_minus_1 / 2.0;
            const double x_var  = (n_values_minus_1 * (n_values_minus_1 + 1) * (2 * n_values_minus_1 + 1)) / (6.0 * n_values) - x_mean * x_mean;
            const double xy_var = xy_sum / n_values - x_mean * (y_sum / n_values);
            slope = (x_var != 0.0 ? xy_var / x_var : 0.0);
#else
            // less readable, but numerically more accurate code relying on integer arithmetics as far as possible
            const size_t x_mean_num = n_values_minus_1;
            const size_t x_mean_den = 2;
            const size_t x_var_num  = n_values_minus_1 * (n_values_minus_1 + 1) * (2 * n_values_minus_1 + 1) * x_mean_den * x_mean_den - x_mean_num * x_mean_num * n_values * 6;
            const size_t x_var_den  = 6 * x_mean_den * x_mean_den * n_values;
            const double xy_var_num = xy_sum * x_mean_den - x_mean_num * y_sum;
            const size_t xy_var_den = x_mean_den * n_values;

            // calculate linear regression
            slope = ((x_var_num * xy_var_den) != 0 ? (xy_var_num * x_var_den) / (x_var_num * xy_var_den) : 0.0);
#endif
        }
    };

}   // namespace libspeedwire

#endif
