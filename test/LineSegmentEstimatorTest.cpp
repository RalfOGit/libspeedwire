#include <gtest/gtest.h>
#include <MeasurementValues.hpp>
#include <LineSegmentEstimator.hpp>

using namespace libspeedwire;

#if 1
// test approximateStepFunctions
TEST(LineSegmentEstimatorTest, approximateStepFunctions) {
    const double noise = 300.0;
    MeasurementValues mv(60);
    for (size_t i = 0; i < mv.getMaximumNumberOfElements() / 2; ++i) {
        double value = 300.0 + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    for (size_t i = mv.getMaximumNumberOfElements() / 2; i < mv.getMaximumNumberOfElements(); ++i) {
        double value = 600.0 + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    std::vector<size_t> steps;
    LineSegmentEstimator::findChangePointsOfMeanValues(mv, steps);
}
#endif

#if 1
// test approximateSlopeFunctions
TEST(LineSegmentEstimatorTest, approximateSlopeFunctions1) {
    const double noise = 300.0;
    MeasurementValues mv(60);
    for (size_t i = 0; i < mv.getMaximumNumberOfElements(); ++i) {
        double value = 300.0 + i * 300.0 + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    std::vector<size_t> steps;
    LineSegmentEstimator::findChangePointsOfMeanValues(mv, steps);
}
#endif

#if 1
// test approximateSlopeFunctions
TEST(LineSegmentEstimatorTest, approximateSlopeFunctions2) {
    const double noise = 300.0;
    MeasurementValues mv(60);
    size_t half_index = mv.getMaximumNumberOfElements() / 2u;
    for (size_t i = 0; i < half_index; ++i) {
        double value = 300.0 + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    for (size_t i = half_index; i < mv.getMaximumNumberOfElements(); ++i) {
        double value = 300.0 + (i - half_index) * 100.0 + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    std::vector<size_t> steps;
    LineSegmentEstimator::findChangePointsOfLinearRegressionValues(mv, steps);
}
#endif

#if 1
// test approximateSlopeFunctions
TEST(LineSegmentEstimatorTest, approximateSlopeFunctions3) {
    const double noise = 100.0;
    const double slope = 100.0;
    MeasurementValues mv(60);
    size_t half_index = mv.getMaximumNumberOfElements() / 2u;
    for (size_t i = 0; i < half_index; ++i) {
        double value = 300.0 + (half_index-1) * slope - i * slope + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    for (size_t i = half_index; i < mv.getMaximumNumberOfElements(); ++i) {
        double value = 300.0 + (i - half_index) * slope + noise * (((double)std::rand() - (RAND_MAX / 2)) / RAND_MAX);
        TimestampDoublePair p(value, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    std::vector<size_t> steps;
    LineSegmentEstimator::findChangePointsOfLinearRegressionValues(mv, steps);
}
#endif