#ifndef __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__
#define __LIBSPEEDWIRE_MEASUREMENTVALUES_HPP__

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
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

    class ChangePoint {
    public:
        size_t change_index_before;
        size_t change_index_after;
    };

    class MeasurementValueInterval {
    public:
        size_t start_index;
        size_t end_index;       // included
        double mean_value;
        MeasurementValueInterval(const size_t start, const size_t end, const double mean) : start_index(start), end_index(end), mean_value(mean) {}
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

        /**
         *  Calculate the average value over the given subset of measurements in the ring buffer.
         *  @param from start index
         *  @param to end index; the measurement with index end is included
         *  @return average value
         */
        double calculateAverageValue(const size_t from, const size_t to) const {
            double sum = 0.0;
            for (size_t index = from; index <= to; ++index) {
                sum += at(index).value;
            }
            return sum / (to - from + 1);
        }

        /**
         *  Find mean value change points by simplified total variation.
         *  @param steps output vector holding indexes of change points; the index points to the last index before the change point
         *  @return the number of change points
         */
        size_t findChangePoints(std::vector<size_t>& steps) const {

            // for each measurement value, estimate mean and variance using a sliding window around the value;
            // the sliding window is from -window_size .. 0 .. window_size
            const size_t window_size = getMaximumNumberOfElements() / 10;   // must be > 0
            std::vector<double> mean; mean.reserve(getMaximumNumberOfElements());
            std::vector<double> variance; variance.reserve(getMaximumNumberOfElements());

            for (size_t i = 0; i < getNumberOfElements(); ++i) {
                const size_t truncated_size = (i < window_size ? i : (i > (getNumberOfElements() - window_size - 1) ? (getNumberOfElements() - i - 1) : window_size));
                const size_t from = i - truncated_size;
                const size_t to   = i + truncated_size;

                double sum = 0.0, squared_sum = 0.0;
                for (size_t w = from; w <= to; ++w) {
                    const double value = at(w).value;
                    sum += value;
                    squared_sum += value * value;
                }
                const double mean_value = sum / (to - from + 1);
                const double var_value = squared_sum / (to - from + 1) - mean_value * mean_value;
                mean.push_back(mean_value);
                variance.push_back(var_value);

                printf("mean %lf  var %lf\n", mean[i], variance[i]);
            }

            // estimate variance values for the oldest and newest measurement value using variances of their direct neighbors
            if (getNumberOfElements() >= 2) {
                variance[0] = variance[1];
                variance[getNumberOfElements() - 1] = variance[getNumberOfElements() - 2];
            }

            // find mean value change points by simplified total variation. This is done by calculating the sum of variances of
            // two adjacent sliding windows. Adjacent sliding windows have their centers 2 * window_size values apart.
            // A change point is characterized by a local minimum sum of variances.
            bool downwards = false;  // minimum seeker state, needed to avoid saddle points
            //for (size_t center_1 = window_size; center_1 < (getNumberOfElements() - 3 * window_size); ++center_1) {
            for (size_t center_1 = 1; center_1 < (getNumberOfElements() - 2 * window_size - 3); ++center_1) {
                const size_t center_2 = center_1 + 2 * window_size + 1;

                // calculate total variation cost functions for this value, the value before and the value after.
                const double penalty_m1 = variance[center_1 - 1] + variance[center_2 - 1];
                const double penalty    = variance[center_1    ] + variance[center_2];
                const double penalty_p1 = variance[center_1 + 1] + variance[center_2 + 1];

                downwards = ((penalty < penalty_m1) ? true : ((penalty > penalty_m1) ? false : downwards));

                if (downwards == true && penalty < penalty_p1) {
                    const size_t min_index = center_1 + window_size;
                    printf("minimum total variation found at %d\n", (int)min_index);

                    // check if mean values differ by more than 3 * sigma; only then this value is considered as a change point;
                    // to avoid square root calculations squared mean differences and 9 * variance are used instead
                    const double mean_diff           = mean[center_1] - mean[center_2];
                    const double mean_diff_squared   = mean_diff * mean_diff;
                    const double three_sigma_squared = 9.0 * 0.5*(variance[center_1] + variance[center_2]);
                    if (mean_diff_squared > three_sigma_squared) {
                        printf("3 sigma total variation minimum found at %d  (mean_diff^2: %lf  9*variance: %lf)\n", (int)min_index, mean_diff_squared, three_sigma_squared);
                        steps.push_back(min_index);
                    }
                }
            }
            return steps.size();
        }

        /**
         *  Find mean value invtervals by simplified total variation.
         *  @param intervals output vector holding interval definitions
         *  @return the number of intervals
         */
        size_t findPiecewiseConstantIntervals(std::vector<MeasurementValueInterval>& intervals) const {
            std::vector<size_t> steps;
            if (findChangePoints(steps) > 0) {
                double avg0 = calculateAverageValue(0, steps[0]);
                intervals.push_back(MeasurementValueInterval(0, steps[0], avg0));

                for (size_t i = 1; i < steps.size(); ++i) {
                    double avg = calculateAverageValue(steps[i - 1] + 1, steps[i]);
                    intervals.push_back(MeasurementValueInterval(steps[i - 1] + 1, steps[i], avg));
                }
                double avgn = calculateAverageValue(steps[steps.size() - 1] + 1, getNumberOfElements() - 1);
                intervals.push_back(MeasurementValueInterval(steps[steps.size() - 1] + 1, getNumberOfElements() - 1, avgn));
            }
            else {
                double avg = calculateAverageValue();
                intervals.push_back(MeasurementValueInterval(0, getNumberOfElements() - 1, avg));
            }
            return intervals.size();
        }

    };

}   // namespace libspeedwire

#endif
