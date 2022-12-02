#include <gtest/gtest.h>
#include <MeasurementValues.hpp>
#include <LineSegmentEstimator.hpp>

using namespace libspeedwire;

static bool approximatelyEqual(double lhs, double rhs) {
    double diff = abs(lhs - rhs);
    return diff < 1e-7;
}

// test index out of bounds methods
TEST(MeasurementValuesTest, IndexOutOfBounds) {
    TimestampDoublePair pair;
    ASSERT_FALSE(MeasurementValues::isIndexOutOfBoundsElement(pair));

    const TimestampDoublePair& outOfBounds = MeasurementValues::getIndexOutOfBoundsElement();
    ASSERT_TRUE(MeasurementValues::isIndexOutOfBoundsElement(outOfBounds));
}

// test number of elements methods
TEST(MeasurementValuesTest, NumberOfElements) {
    MeasurementValues mv0(0);
    MeasurementValues mv1(1);
    MeasurementValues mv2(2);
    MeasurementValues mv3(3);
    TimestampDoublePair pair(1.0, 1000);

    // empty buffer with cacpacity set in constructor
    ASSERT_EQ(mv0.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(mv1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(mv3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(mv0.getNumberOfElements(), 0);
    ASSERT_EQ(mv1.getNumberOfElements(), 0);
    ASSERT_EQ(mv2.getNumberOfElements(), 0);
    ASSERT_EQ(mv3.getNumberOfElements(), 0);

    // empty buffer with cacpacity set explicitly
    mv0.setMaximumNumberOfElements(0);
    mv1.setMaximumNumberOfElements(1);
    mv2.setMaximumNumberOfElements(2);
    mv3.setMaximumNumberOfElements(3);
    ASSERT_EQ(mv0.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(mv1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(mv3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(mv0.getNumberOfElements(), 0);
    ASSERT_EQ(mv1.getNumberOfElements(), 0);
    ASSERT_EQ(mv2.getNumberOfElements(), 0);
    ASSERT_EQ(mv3.getNumberOfElements(), 0);

    // add one element
    mv0.addNewElement(pair);
    mv1.addNewElement(pair);
    mv2.addNewElement(pair);
    mv3.addNewElement(pair);
    ASSERT_EQ(mv0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(mv3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(mv0.getNumberOfElements(), 1);
    ASSERT_EQ(mv1.getNumberOfElements(), 1);
    ASSERT_EQ(mv2.getNumberOfElements(), 1);
    ASSERT_EQ(mv3.getNumberOfElements(), 1);

    // add second element
    mv0.addNewElement(pair);
    mv1.addNewElement(pair);
    mv2.addNewElement(pair);
    mv3.addNewElement(pair);
    ASSERT_EQ(mv0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(mv3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(mv0.getNumberOfElements(), 1);
    ASSERT_EQ(mv1.getNumberOfElements(), 1);
    ASSERT_EQ(mv2.getNumberOfElements(), 2);
    ASSERT_EQ(mv3.getNumberOfElements(), 2);

    // add third element
    mv0.addNewElement(pair);
    mv1.addNewElement(pair);
    mv2.addNewElement(pair);
    mv3.addNewElement(pair);
    ASSERT_EQ(mv0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(mv3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(mv0.getNumberOfElements(), 1);
    ASSERT_EQ(mv1.getNumberOfElements(), 1);
    ASSERT_EQ(mv2.getNumberOfElements(), 2);
    ASSERT_EQ(mv3.getNumberOfElements(), 3);

    // add fourth element
    mv0.addNewElement(pair);
    mv1.addNewElement(pair);
    mv2.addNewElement(pair);
    mv3.addNewElement(pair);
    ASSERT_EQ(mv0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(mv2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(mv3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(mv0.getNumberOfElements(), 1);
    ASSERT_EQ(mv1.getNumberOfElements(), 1);
    ASSERT_EQ(mv2.getNumberOfElements(), 2);
    ASSERT_EQ(mv3.getNumberOfElements(), 3);
}

// test zero capacity
TEST(MeasurementValuesTest, Capacity0) {
    MeasurementValues mv(0);
    ASSERT_EQ(mv.findClosestIndex(0), (size_t)-1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, TimestampDoublePair::defaultPair.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, TimestampDoublePair::defaultPair.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), 0.0);
}

// test capacity of one
TEST(MeasurementValuesTest, Capacity1) {
    MeasurementValues mv(1);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);

    // empty buffer with cacpacity set in constructor
    ASSERT_EQ(mv.findClosestIndex(0), (size_t)-1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, TimestampDoublePair::defaultPair.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, TimestampDoublePair::defaultPair.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), 0.0);

    // empty buffer with explicitly set capacity
    mv.setMaximumNumberOfElements(1);
    ASSERT_EQ(mv.findClosestIndex(0), (size_t)-1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, TimestampDoublePair::defaultPair.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, TimestampDoublePair::defaultPair.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), 0.0);

    // buffer with one element 1
    mv.addNewElement(pair1);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(2000), 0);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair1.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), pair1.value);

    // buffer with replaced element 2
    mv.addNewElement(pair2);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(2000), 0);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair2.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair2.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), pair2.value);
}

// test capacity of two
TEST(MeasurementValuesTest, Capacity2) {
    MeasurementValues mv(2);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);
    TimestampDoublePair pair3(3.0, 3000);

    // empty buffer with cacpacity set in constructor
    ASSERT_EQ(mv.findClosestIndex(0), (size_t)-1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, TimestampDoublePair::defaultPair.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, TimestampDoublePair::defaultPair.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), 0.0);

    // buffer with one element 1
    mv.addNewElement(pair1);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(2000), 0);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair1.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), pair1.value);

    // buffer with two elements 1 and 2
    mv.addNewElement(pair2);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(1499), 0);
    ASSERT_EQ(mv.findClosestIndex(1501), 1);
    ASSERT_EQ(mv.findClosestIndex(2000), 1);
    ASSERT_EQ(mv.findClosestIndex(3000), 1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(3000).value, pair2.value);
    ASSERT_EQ(mv.findClosestMeasurement(3000).time, pair2.time);
    ASSERT_EQ(mv.interpolateClosestValues(1000), pair1.value);
    ASSERT_EQ(mv.interpolateClosestValues(2000), pair2.value);
    ASSERT_EQ(mv.interpolateClosestValues(1500), (pair1.value + pair2.value) / 2);
    ASSERT_EQ(mv.interpolateClosestValues(1250), (3*pair1.value + pair2.value) / 4);
    ASSERT_EQ(mv.interpolateClosestValues(1750), (pair1.value + 3 * pair2.value) / 4);

    // buffer with two elements 2 and 3
    mv.addNewElement(pair3);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(2000), 0);
    ASSERT_EQ(mv.findClosestIndex(2499), 0);
    ASSERT_EQ(mv.findClosestIndex(2501), 1);
    ASSERT_EQ(mv.findClosestIndex(3000), 1);
    ASSERT_EQ(mv.findClosestIndex(4000), 1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair2.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair2.time);
    ASSERT_EQ(mv.findClosestMeasurement(4000).value, pair3.value);
    ASSERT_EQ(mv.findClosestMeasurement(4000).time, pair3.time);
    ASSERT_EQ(mv.interpolateClosestValues(2000), pair2.value);
    ASSERT_EQ(mv.interpolateClosestValues(3000), pair3.value);
    ASSERT_EQ(mv.interpolateClosestValues(2500), (pair2.value + pair3.value) / 2);
    ASSERT_EQ(mv.interpolateClosestValues(2250), (3 * pair2.value + pair3.value) / 4);
    ASSERT_EQ(mv.interpolateClosestValues(2750), (pair2.value + 3 * pair3.value) / 4);
}

// test capacity of three
TEST(MeasurementValuesTest, Capacity3) {
    MeasurementValues mv(3);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);
    TimestampDoublePair pair3(3.0, 3000);
    TimestampDoublePair pair4(4.0, 4000);

    // empty buffer with cacpacity set in constructor
    ASSERT_EQ(mv.findClosestIndex(0), (size_t)-1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, TimestampDoublePair::defaultPair.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, TimestampDoublePair::defaultPair.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), 0.0);

    // buffer with one element 1
    mv.addNewElement(pair1);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(2000), 0);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(1000).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(1000).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(2000).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(2000).time, pair1.time);
    ASSERT_EQ(mv.interpolateClosestValues(0), pair1.value);

    // buffer with two elements 1 and 2
    mv.addNewElement(pair2);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(1499), 0);
    ASSERT_EQ(mv.findClosestIndex(1501), 1);
    ASSERT_EQ(mv.findClosestIndex(2000), 1);
    ASSERT_EQ(mv.findClosestIndex(3000), 1);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(1000).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(1000).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(2000).value, pair2.value);
    ASSERT_EQ(mv.findClosestMeasurement(2000).time, pair2.time);
    ASSERT_EQ(mv.findClosestMeasurement(3000).value, pair2.value);
    ASSERT_EQ(mv.findClosestMeasurement(3000).time, pair2.time);
    ASSERT_EQ(mv.interpolateClosestValues(1000), pair1.value);
    ASSERT_EQ(mv.interpolateClosestValues(2000), pair2.value);
    ASSERT_EQ(mv.interpolateClosestValues(1500), (pair1.value + pair2.value) / 2);
    ASSERT_EQ(mv.interpolateClosestValues(1250), (3 * pair1.value + pair2.value) / 4);
    ASSERT_EQ(mv.interpolateClosestValues(1750), (pair1.value + 3 * pair2.value) / 4);

    // buffer with two elements 1, 2 and 3
    mv.addNewElement(pair3);
    ASSERT_EQ(mv.findClosestIndex(-1), 0);
    ASSERT_EQ(mv.findClosestIndex(0), 0);
    ASSERT_EQ(mv.findClosestIndex(1000), 0);
    ASSERT_EQ(mv.findClosestIndex(2000), 1);
    ASSERT_EQ(mv.findClosestIndex(2499), 1);
    ASSERT_EQ(mv.findClosestIndex(2501), 2);
    ASSERT_EQ(mv.findClosestIndex(3000), 2);
    ASSERT_EQ(mv.findClosestIndex(4000), 2);
    ASSERT_EQ(mv.findClosestMeasurement(0).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(0).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(1000).value, pair1.value);
    ASSERT_EQ(mv.findClosestMeasurement(1000).time, pair1.time);
    ASSERT_EQ(mv.findClosestMeasurement(2000).value, pair2.value);
    ASSERT_EQ(mv.findClosestMeasurement(2000).time, pair2.time);
    ASSERT_EQ(mv.findClosestMeasurement(3000).value, pair3.value);
    ASSERT_EQ(mv.findClosestMeasurement(3000).time, pair3.time);
    ASSERT_EQ(mv.findClosestMeasurement(4000).value, pair3.value);
    ASSERT_EQ(mv.findClosestMeasurement(4000).time, pair3.time);
    ASSERT_EQ(mv.interpolateClosestValues(2000), pair2.value);
    ASSERT_EQ(mv.interpolateClosestValues(3000), pair3.value);
    ASSERT_EQ(mv.interpolateClosestValues(1500), (pair1.value + pair2.value) / 2);
    ASSERT_EQ(mv.interpolateClosestValues(1250), (3 * pair1.value + pair2.value) / 4);
    ASSERT_EQ(mv.interpolateClosestValues(1750), (pair1.value + 3 * pair2.value) / 4);
    ASSERT_EQ(mv.interpolateClosestValues(2500), (pair2.value + pair3.value) / 2);
    ASSERT_EQ(mv.interpolateClosestValues(2250), (3 * pair2.value + pair3.value) / 4);
    ASSERT_EQ(mv.interpolateClosestValues(2750), (pair2.value + 3 * pair3.value) / 4);
}

// test findClosestIndex with a capacity of 60
TEST(MeasurementValuesTest, FindClosestMeasurement) {
    MeasurementValues mv(60);
    for (size_t i = 0; i < mv.getMaximumNumberOfElements(); ++i) {
        TimestampDoublePair p((double)i, (uint32_t)(i * 1000));
        mv.addNewElement(p);
    }
    for (size_t i = 0; i < 1000 * mv.getNumberOfElements() - 500; ++i) {
        if ((i % 500) == 0) {
            const size_t index = mv.findClosestIndex((uint32_t)i);
            ASSERT_TRUE(index == (i + 500) / 1000 || index == (i + 499) / 1000);
        }
        else {
            ASSERT_EQ(mv.findClosestIndex((uint32_t)i), (i + 500) / 1000);
        }
    }
    for (int i = -5000; i <= 0; ++i) {
        ASSERT_EQ(mv.findClosestIndex((uint32_t)i), 0);
    }
    for (size_t i = 1000 * mv.getNumberOfElements(); i < 1005 * mv.getNumberOfElements(); ++i) {
        ASSERT_EQ(mv.findClosestIndex((uint32_t)i), mv.getNumberOfElements()-1);
    }
}

// test capacity of one - calculateSampleMean
TEST(MeasurementValuesTest, Capacity1_calculateSampleMean) {
    MeasurementValues mv(1);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);

    mv.addNewElement(pair1);
    ASSERT_EQ(mv.estimateMean(), pair1.value);
    ASSERT_EQ(mv.estimateMean(0, 0), pair1.value);

    mv.addNewElement(pair2);
    ASSERT_EQ(mv.estimateMean(), pair2.value);
    ASSERT_EQ(mv.estimateMean(0, 0), pair2.value);
}

// test capacity of two - estimateMean
TEST(MeasurementValuesTest, Capacity2_estimateMean) {
    MeasurementValues mv(2);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);
    TimestampDoublePair pair3(3.0, 3000);

    mv.addNewElement(pair1);
    ASSERT_EQ(mv.estimateMean(), pair1.value);
    ASSERT_EQ(mv.estimateMean(0, 0), pair1.value);

    mv.addNewElement(pair2);
    ASSERT_EQ(mv.estimateMean(), (pair1.value + pair2.value) / 2);
    ASSERT_EQ(mv.estimateMean(0, 1), (pair1.value + pair2.value) / 2);

    mv.addNewElement(pair3);
    ASSERT_EQ(mv.estimateMean(), (pair2.value + pair3.value) / 2);
    ASSERT_EQ(mv.estimateMean(0, 1), (pair2.value + pair3.value) / 2);
}

// test capacity of three - estimateMean
TEST(MeasurementValuesTest, Capacity3_estimateMean) {
    MeasurementValues mv(3);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);
    TimestampDoublePair pair3(3.0, 3000);
    TimestampDoublePair pair4(4.0, 4000);

    mv.addNewElement(pair1);
    ASSERT_EQ(mv.estimateMean(), pair1.value);
    ASSERT_EQ(mv.estimateMean(0, 0), pair1.value);

    mv.addNewElement(pair2);
    ASSERT_EQ(mv.estimateMean(), (pair1.value + pair2.value) / 2);
    ASSERT_EQ(mv.estimateMean(0, 1), (pair1.value + pair2.value) / 2);

    mv.addNewElement(pair3);
    ASSERT_EQ(mv.estimateMean(), (pair1.value + pair2.value + pair3.value) / 3);
    ASSERT_EQ(mv.estimateMean(0, 2), (pair1.value + pair2.value + pair3.value) / 3);

    mv.addNewElement(pair4);
    ASSERT_EQ(mv.estimateMean(), (pair2.value + pair3.value + pair4.value) / 3);
    ASSERT_EQ(mv.estimateMean(0, 2), (pair2.value + pair3.value + pair4.value) / 3);
}

// test capacity of three - estimateMeanAndVariance
TEST(MeasurementValuesTest, Capacity3_estimateMeanAndVariance) {
    MeasurementValues mv(3);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);
    TimestampDoublePair pair3(3.0, 3000);
    TimestampDoublePair pair4(4.0, 4000);
    double mean, variance;

    mv.addNewElement(pair1);
    mv.estimateMeanAndVariance(0, 0, mean, variance);
    ASSERT_EQ(mean, pair1.value);
    ASSERT_EQ(variance, FLT_MAX);

    mv.addNewElement(pair2);
    mv.estimateMeanAndVariance(0, 1, mean, variance);
    ASSERT_EQ(mean, (pair1.value + pair2.value) / 2);
    ASSERT_EQ(variance, 0.5);

    mv.addNewElement(pair3);
    mv.estimateMeanAndVariance(0, 2, mean, variance);
    ASSERT_EQ(mean, (pair1.value + pair2.value + pair3.value) / 3);
    ASSERT_EQ(variance, 1.0);

    mv.addNewElement(pair4);
    mv.estimateMeanAndVariance(0, 2, mean, variance);
    ASSERT_EQ(mean, (pair2.value + pair3.value + pair4.value) / 3);
    ASSERT_EQ(variance, 1.0);
}

// test capacity of three - estimateLinearRegression
TEST(MeasurementValuesTest, Capacity3_calculateLinearRegression) {
    MeasurementValues mv(3);
    TimestampDoublePair pair1(1.0, 1000);
    TimestampDoublePair pair2(2.0, 2000);
    TimestampDoublePair pair3(3.0, 3000);
    TimestampDoublePair pair4(4.0, 4000);
    double mean, slope, variance;

    mv.addNewElement(pair1);
    mv.estimateLinearRegression(0, 0, mean, variance, slope);
    ASSERT_EQ(mean, pair1.value);
    ASSERT_EQ(variance, FLT_MAX);
    ASSERT_EQ(slope, 0.0);

    mv.addNewElement(pair2);
    mv.estimateLinearRegression(0, 1, mean, variance, slope);
    ASSERT_EQ(mean, (pair1.value + pair2.value) / 2);
    ASSERT_EQ(variance, 0.5);
    EXPECT_DOUBLE_EQ(slope, 1.0);

    mv.addNewElement(pair3);
    mv.estimateLinearRegression(0, 2, mean, variance, slope);
    ASSERT_EQ(mean, (pair1.value + pair2.value + pair3.value) / 3);
    ASSERT_EQ(variance, 1.0);
    EXPECT_DOUBLE_EQ(slope, 1.0);

    mv.addNewElement(pair4);
    mv.estimateLinearRegression(0, 2, mean, variance, slope);
    ASSERT_EQ(mean, (pair2.value + pair3.value + pair4.value) / 3);
    ASSERT_EQ(variance, 1.0);
    EXPECT_DOUBLE_EQ(slope, 1.0);
}

// test capacity of three - estimateLinearRegression
TEST(MeasurementValuesTest, Capacity3_estimateLinearRegressionDown) {
    MeasurementValues mv(3);
    TimestampDoublePair pair1(4.0, 1000);
    TimestampDoublePair pair2(3.0, 2000);
    TimestampDoublePair pair3(2.0, 3000);
    TimestampDoublePair pair4(1.0, 4000);
    double mean, slope, variance;

    mv.addNewElement(pair1);
    mv.estimateLinearRegression(0, 0, mean, variance, slope);
    ASSERT_EQ(mean, pair1.value);
    ASSERT_EQ(variance, FLT_MAX);
    ASSERT_EQ(slope, 0.0);

    mv.addNewElement(pair2);
    mv.estimateLinearRegression(0, 1, mean, variance, slope);
    ASSERT_EQ(mean, (pair1.value + pair2.value) / 2);
    ASSERT_EQ(variance, 0.5);
    EXPECT_DOUBLE_EQ(slope, -1.0);

    mv.addNewElement(pair3);
    mv.estimateLinearRegression(0, 2, mean, variance, slope);
    ASSERT_EQ(mean, (pair1.value + pair2.value + pair3.value) / 3);
    ASSERT_EQ(variance, 1.0);
    EXPECT_DOUBLE_EQ(slope, -1.0);

    mv.addNewElement(pair4);
    mv.estimateLinearRegression(0, 2, mean, variance, slope);
    ASSERT_EQ(mean, (pair2.value + pair3.value + pair4.value) / 3);
    ASSERT_EQ(variance, 1.0);
    EXPECT_DOUBLE_EQ(slope, -1.0);
}
