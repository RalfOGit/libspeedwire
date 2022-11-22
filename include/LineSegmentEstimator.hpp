#ifndef __LIBSPEEDWIRE_LINESEGMENTESTIMATOR_HPP__
#define __LIBSPEEDWIRE_LINESEGMENTESTIMATOR_HPP__

#include <cstdint>
#include <cstdlib>
#include <vector>
#include <MeasurementValues.hpp>


namespace libspeedwire {

    //struct ChangePoint {
    //    size_t change_index_before;
    //    size_t change_index_after;
    //};

    struct MeasurementValueInterval {
        size_t start_index;
        size_t end_index;       // included
        double mean_value;
        MeasurementValueInterval(const size_t start, const size_t end, const double mean) : start_index(start), end_index(end), mean_value(mean) {}
    };

    struct StatisticalEstimates {
        double mean;            //!< mean value, i.e.the arithmetic average
        double variance;        //!< variance with respect to the mean value
        double slope;           //!< slope value, i.e. the slope value of a straight line through P(x, y) = (0, mean)
        double sloped_variance; //!< slope variance, i.e.the squared y-component diffs to the straight line defined by slope and intercept point (0, mean)
        StatisticalEstimates(const double m, const double var, const double sl, const double sl_var) : mean(m), variance(var), slope(sl), sloped_variance(sl_var) {}
    };

    class LineSegmentEstimator {
    public:

        /**
         *  Find mean value change points by simplified total variation.
         *  @param steps output vector holding indexes of change points; the index points to the last index before the change point
         *  @return the number of change points
         */
        static size_t findChangePointsOfMeanValues(const MeasurementValues& mvalues, std::vector<size_t>& steps) {
            const size_t window_size = mvalues.getMaximumNumberOfElements() / 10;   // must be > 0
            const bool enable_linear_regression = false;

            std::vector<StatisticalEstimates> estimates;
            estimates.reserve(mvalues.getMaximumNumberOfElements());

            // for each measurement value, estimate statistical parameters in a sliding window around the value;
            // the sliding window size is: -window_size .. 0 .. window_size
            estimateStatistics(mvalues, window_size, enable_linear_regression, estimates);

            // find mean value change points by simplified total variation. This is done by calculating the sum of variances of
            // two adjacent sliding windows. Adjacent sliding windows have their centers 2 * window_size values apart.
            // A change point is characterized by a local minimum sum of variances.
            return totalVariationOfMeanValues(mvalues, window_size, estimates, steps);
        }

        /**
         *  Find linear regression change points by simplified total variation.
         *  @param steps output vector holding indexes of change points; the index points to the last index before the change point
         *  @return the number of change points
         */
        static size_t findChangePointsOfLinearRegressionValues(const MeasurementValues& mvalues, std::vector<size_t>& steps) {
            const size_t window_size = mvalues.getMaximumNumberOfElements() / 10;   // must be > 0
            const bool enable_linear_regression = true;

            std::vector<StatisticalEstimates> estimates;
            estimates.reserve(mvalues.getMaximumNumberOfElements());

            // for each measurement value, estimate statistical parameters in a sliding window around the value;
            // the sliding window size is: -window_size .. 0 .. window_size
            estimateStatistics(mvalues, window_size, enable_linear_regression, estimates);

            // find linear regression change points by simplified total variation. This is done by calculating the sum of variances of
            // two adjacent sliding windows. Adjacent sliding windows have their centers 2 * window_size values apart.
            // A change point is characterized by a local minimum sum of variances.
            return totalVariationOfLinearRegressionValues(mvalues, window_size, estimates, steps);
        }

        /**
         *  Find mean value intervals by simplified total variation.
         *  @param intervals output vector holding interval definitions
         *  @return the number of intervals
         */
        static size_t findPiecewiseConstantIntervals(const MeasurementValues& mvalues, std::vector<MeasurementValueInterval>& intervals) {
            std::vector<size_t> steps;
            if (findChangePointsOfMeanValues(mvalues, steps) > 0) {
                double avg0 = mvalues.estimateMean(0, steps[0]);
                intervals.push_back(MeasurementValueInterval(0, steps[0], avg0));

                for (size_t i = 1; i < steps.size(); ++i) {
                    double avg = mvalues.estimateMean(steps[i - 1] + 1, steps[i]);
                    intervals.push_back(MeasurementValueInterval(steps[i - 1] + 1, steps[i], avg));
                }
                double avgn = mvalues.estimateMean(steps[steps.size() - 1] + 1, mvalues.getNumberOfElements() - 1);
                intervals.push_back(MeasurementValueInterval(steps[steps.size() - 1] + 1, mvalues.getNumberOfElements() - 1, avgn));
            }
            else {
                double avg = mvalues.estimateMean();
                intervals.push_back(MeasurementValueInterval(0, mvalues.getNumberOfElements() - 1, avg));
            }
            return intervals.size();
        }

    protected:

        /**
         *  Estimate statistical parameters for each values in the given measurement values.
         *  A sliding window around each value is used to estimate:
         *  - mean value, i.e. the arithmetic average of the values inside the sliding window
         *  - variance to mean value, i.e. the squared diff to of the values inside the sliding window to the mean value
         *  - slope value, i.e. the slope value determined by linear regression (optional)
         *  - slope variance, i.e. the squared diff of the values y component to the line defined by slope and mean as intercept point (optional)
         *  @param mvalues input measurement values
         *  @param window_size the sliding window size: -window_size .. 0 .. window_size
         *  @param enable_linear_regression enable estimation of optional linear regression parameters (slope and variance to slope)
         *  @param estimates output statistical parameters
         */
        static void estimateStatistics(const MeasurementValues& mvalues, const size_t window_size, const bool enable_linear_regression, std::vector<StatisticalEstimates>& estimates) {
            const size_t num_values = mvalues.getMaximumNumberOfElements();

            // for each measurement value, estimate statistical parameters in a sliding window around the value
            for (size_t i = 0; i < num_values; ++i) {
                const size_t truncated_size = (i < window_size ? i : (i > (num_values - window_size - 1) ? (num_values - i - 1) : window_size));
                const size_t from = i - truncated_size;
                const size_t to   = i + truncated_size;
                if (enable_linear_regression == false) {
                    double y_mean, y_var;
                    mvalues.estimateMeanAndVariance(from, to, y_mean, y_var);
                    estimates.push_back(StatisticalEstimates(y_mean, y_var, 0.0, y_var));
                }
                else {
                    double y_mean, y_var, slope;
                    mvalues.estimateLinearRegression(from, to, y_mean, y_var, slope);
                    estimates.push_back(StatisticalEstimates(y_mean, y_var, slope, y_var));
                }
            }
            // estimate variance values for the oldest and newest measurement value using variances of their direct neighbors
            if (num_values >= 2) {
                estimates[0].variance        = 1000.0 * estimates[1].variance;
                estimates[0].sloped_variance = 1000.0 * estimates[1].sloped_variance;
                estimates[num_values - 1].variance        = 1000.0 * estimates[num_values - 2].variance;
                estimates[num_values - 1].sloped_variance = 1000.0 * estimates[num_values - 2].sloped_variance;
            }
#if defined(_DEBUG)
            int i = 0;
            for (const auto& estim : estimates) {
                printf("i %02d  value %lf  mean %lf  var %lf  slope %lf  slope_var %lf\n", (int)i, mvalues.at(i).value, estim.mean, estim.variance, estim.slope, estim.sloped_variance); ++i;
            }
#endif
        }

        /**
         *  Find mean value change points by simplified total variation. This is done by calculating the sum of variances of
         *  two adjacent sliding windows. Adjacent sliding windows have their centers 2 * window_size values apart.
         *  A change point is characterized by a local minimum sum of variances.
         *  @param mvalues input measurement values
         *  @param window_size the sliding window size: -window_size .. 0 .. window_size
         *  @param estimates input statistical parameters
         *  @param steps
         *  @return number of steps
         */
        static size_t totalVariationOfMeanValues(const MeasurementValues& mvalues, const size_t window_size, const std::vector<StatisticalEstimates>& estimates, std::vector<size_t>& steps) {

            // find mean value change points by simplified total variation. This is done by calculating the sum of variances of
            // two adjacent sliding windows. Adjacent sliding windows have their centers 2 * window_size values apart.
            // A change point is characterized by a local minimum sum of variances.
            bool downwards = false;  // minimum seeker state, needed to avoid saddle points
            for (size_t center_1 = 1; center_1 < (mvalues.getNumberOfElements() - 2 * window_size - 3); ++center_1) {
                const size_t center_2 = center_1 + 2 * window_size + 1;

                // calculate total variation cost functions for this value, the value before and the value after.
                const double penalty_m1 = estimates[center_1 - 1].variance + estimates[center_2 - 1].variance;
                const double penalty    = estimates[center_1    ].variance + estimates[center_2    ].variance;
                const double penalty_p1 = estimates[center_1 + 1].variance + estimates[center_2 + 1].variance;

                downwards = ((penalty < penalty_m1) ? true : ((penalty > penalty_m1) ? false : downwards));

                if (downwards == true && penalty < penalty_p1) {
                    const size_t min_index = center_1 + window_size;
                    //printf("minimum total variation found at %d\n", (int)min_index);

                    // check if mean values differ by more than 3 * sigma; only then this value is considered as a change point;
                    // to avoid square root calculations squared mean differences and 9 * variance are used instead
                    const double mean_diff = estimates[center_1].mean - estimates[center_2].mean;
                    const double mean_diff_squared = mean_diff * mean_diff;
                    const double three_sigma_squared = 9.0 * 0.5 * (estimates[center_1].variance + estimates[center_2].variance);
                    if (mean_diff_squared > three_sigma_squared) {
                        // ignore if the variance is small compared to the absolute value
                        if (three_sigma_squared > 200.0) {
#ifdef _DEBUG
                            printf("3 sigma total variation minimum found at %d  (mean_1 %lf  mean_2 %lf  mean_diff^2: %lf  9*variance: %lf)\n", (int)min_index, estimates[center_1].mean, estimates[center_2].mean, mean_diff_squared, three_sigma_squared);
#endif
                            steps.push_back(min_index);
                        }
#ifdef _DEBUG
                        else {
                            printf("3 sigma total variation minimum ignored at %d  (mean_1 %lf  mean_2 %lf  mean_diff^2: %lf  9*variance: %lf)\n", (int)min_index, estimates[center_1].mean, estimates[center_2].mean, mean_diff_squared, three_sigma_squared);
                        }
#endif
                    }
                }
            }
            return steps.size();
        }

        /**
         *  Find linear regression change points by simplified total variation. This is done by calculating the sum of variances of
         *  two adjacent sliding windows. Adjacent sliding windows have their centers 2 * window_size values apart.
         *  A change point is characterized by a local minimum sum of variances.
         *  @param mvalues input measurement values
         *  @param window_size the sliding window size: -window_size .. 0 .. window_size
         *  @param estimates input statistical parameters
         *  @param change_points
         *  @return number of steps
         */
        static size_t totalVariationOfLinearRegressionValues(const MeasurementValues& mvalues, const size_t window_size, const std::vector<StatisticalEstimates>& estimates, std::vector<size_t>& change_points) {
            const size_t num_values = mvalues.getMaximumNumberOfElements();
            const size_t min_window = 2 * window_size;

            // find local minima of cost. This is done by calculating the sum of variances of two adjacent sliding windows.
            // Adjacent sliding windows have their centers 2 * window_size values apart.
            struct MinEntry {
                size_t index;
                double cost;
                MinEntry(const size_t i, const double c) : index(i), cost(c) {}
            };
            std::vector<MinEntry> minima;
            minima.reserve(num_values / 2u);
            for (size_t center_1 = 0; center_1 < (num_values - 2 * window_size - 1); ++center_1) {
                const size_t center_m = center_1 + window_size;
                const size_t center_2 = center_1 + 2 * window_size + 1;
                const double cost = estimates[center_1].sloped_variance + estimates[center_2].sloped_variance;
                const size_t minima_size = minima.size();
                //if (minima_size > 0) printf("cost %lf center_m %lu  prev_cost %lf prev_center_m %lu\n", cost, (unsigned)center_m, minima[minima_size - 1].cost, (unsigned)minima[minima_size - 1].index);
                if (minima_size == 0) {
                    //printf("=> cost %lf center_m %lu  prev_cost - prev_center_m -\n", cost, (unsigned)center_m);
                    minima.push_back(MinEntry(center_m, cost));
                }
                else if (cost < minima[minima_size - 1].cost && center_m > minima[minima_size - 1].index + min_window) {
                    //printf("=> cost %lf center_m %lu  prev_cost %lf prev_center_m %lu\n", cost, (unsigned)center_m, minima[minima_size - 1].cost, (unsigned)minima[minima_size - 1].index);
                    minima.push_back(MinEntry(center_m, cost));
                }
            }

            // find local minima of cost
            for (size_t i = 0; i < minima.size(); ++i) {
                const size_t min_index = minima[i].index;
                const double min_cost = minima[i].cost;
                const size_t center_1 = min_index - window_size;
                const size_t center_m = min_index;
                const size_t center_2 = min_index + window_size + 1;

                // check if mean values extrapolated to the other mean value differ by more than 3 * sigma; 
                // only then this value is considered as a change point;
                // to avoid square root calculations squared mean differences and 9 * variance are used instead
                const double mean_12 = estimates[center_1].mean + estimates[center_1].slope * (2 * window_size + 1);
                const double mean_21 = estimates[center_2].mean - estimates[center_2].slope * (2 * window_size + 1);
                const double mean_diff_squared = (mean_12 - mean_21) * (mean_12 - mean_21);
                const double three_sigma_squared = 9.0 * 0.5 * (estimates[center_1].sloped_variance + estimates[center_2].sloped_variance);
                //printf("minimum total variation found at %d  cost %lf  mean_1 %lf slope_1 %lf  mean_2 %lf slope_2 %lf  mean_12 %lf mean_21 %lf\n", (int)center_m, min_cost, estimates[center_1].mean, estimates[center_1].slope, estimates[center_2].mean, estimates[center_2].slope, mean_12, mean_21);
                if (mean_diff_squared > three_sigma_squared) {
                    // ignore if the variance is small compared to the absolute value
#ifdef _DEBUG
                    printf("3 sigma total variation minimum found at %d  (mean_12 %lf  mean_21 %lf  mean_diff^2: %lf  9*variance: %lf)\n", (int)center_m, mean_12, mean_21, mean_diff_squared, three_sigma_squared);
#endif
                    change_points.push_back(center_m);
                }
            }

            return change_points.size();
        }
    };

}   // namespace libspeedwire

#endif
