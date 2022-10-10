#include <gtest/gtest.h>
#include <RingBuffer.hpp>
#include <MeasurementValues.hpp>

using namespace libspeedwire;

// test index out of bounds
TEST(RingBufferTest, IndexOutOfBounds) {
    TimestampDoublePair pair(0.0, 0);
    ASSERT_FALSE(RingBuffer<TimestampDoublePair>::isIndexOutOfBoundsElement(pair));
    ASSERT_FALSE(RingBuffer<TimestampDoublePair>::isIndexOutOfBoundsElement(pair));
    
    const TimestampDoublePair& outOfBounds = RingBuffer<TimestampDoublePair>::getIndexOutOfBoundsElement();
    ASSERT_TRUE(RingBuffer<TimestampDoublePair>::isIndexOutOfBoundsElement(outOfBounds));
}

// test zero capacity
TEST(RingBufferTest, Capacity0) {
    auto rb = RingBuffer<TimestampDoublePair>(0);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(rb.getNumberOfElements(), 0);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), -1);
    ASSERT_EQ(rb.getDataVectorIndex(1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), -1);
    ASSERT_EQ(rb.getRingBufferIndex(1), -1);
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[ 0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[ 1]));
}

// test capacity of one
TEST(RingBufferTest, Capacity1) {
    // empty buffer with cacpacity set in constructor
    auto rb = RingBuffer<TimestampDoublePair>(1);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb.getNumberOfElements(), 0);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[1]));

    // empty buffer with explicitly set capacity
    rb.setMaximumNumberOfElements(1);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb.getNumberOfElements(), 0);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), -1);
    ASSERT_EQ(rb.getDataVectorIndex(1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), -1);
    ASSERT_EQ(rb.getRingBufferIndex(1), -1);
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[1]));

    // buffer with one element p1(1.0, 1)
    TimestampDoublePair p1(1.0, 1);
    rb.addNewElement(p1);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb.getNumberOfElements(), 1);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), 0);
    ASSERT_EQ(rb.getDataVectorIndex(1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), 0);
    ASSERT_EQ(rb.getRingBufferIndex(1), -1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE (rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE (rb.isIndexOutOfBoundsElement(rb[1]));
    ASSERT_EQ(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(rb.getNewestElement().value, p1.value);

    // buffer with replaced element p2(1.0, 1)
    TimestampDoublePair p2(2.0, 2);
    rb.addNewElement(p2);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb.getNumberOfElements(), 1);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), 0);
    ASSERT_EQ(rb.getDataVectorIndex(1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), 0);
    ASSERT_EQ(rb.getRingBufferIndex(1), -1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[1]));
    ASSERT_EQ(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(rb.getNewestElement().value, p2.value);
}

// test capacity of two
TEST(RingBufferTest, Capacity2) {
    // empty buffer with cacpacity set in constructor
    auto rb = RingBuffer<TimestampDoublePair>(2);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb.getNumberOfElements(), 0);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[1]));

    // empty buffer with explicitly set capacity
    rb.setMaximumNumberOfElements(2);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb.getNumberOfElements(), 0);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), -1);
    ASSERT_EQ(rb.getDataVectorIndex(1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), -1);
    ASSERT_EQ(rb.getRingBufferIndex(1), -1);
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[1]));

    // buffer with one element p1(1.0, 1)
    TimestampDoublePair p1(1.0, 1);
    rb.addNewElement(p1);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb.getNumberOfElements(), 1);
    ASSERT_EQ(rb.getWritePointer(), 1);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), 0);
    ASSERT_EQ(rb.getDataVectorIndex(1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), 0);
    ASSERT_EQ(rb.getRingBufferIndex(1), -1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[1]));
    ASSERT_EQ(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(rb.getNewestElement().value, p1.value);

    // buffer with two elements p1(1.0, 1) and p2(2.0, 2)
    TimestampDoublePair p2(2.0, 2);
    rb.addNewElement(p2);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb.getNumberOfElements(), 2);
    ASSERT_EQ(rb.getWritePointer(), 0);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), 0);
    ASSERT_EQ(rb.getDataVectorIndex(1), 1);
    ASSERT_EQ(rb.getDataVectorIndex(2), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), 0);
    ASSERT_EQ(rb.getRingBufferIndex(1), 1);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(0)), 0);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(1)), 1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[1]));
    ASSERT_NE(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_NE(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(&rb.getOldestElement(), &rb[0]);
    ASSERT_EQ(&rb.getNewestElement(), &rb[1]);
    ASSERT_NE(&rb.getOldestElement(), &rb[1]);
    ASSERT_EQ(rb.getOldestElement().value, p1.value);
    ASSERT_EQ(rb.getNewestElement().value, p2.value);

    // buffer with two elements p2(2.0, 2) and p3(3.0, 3)
    TimestampDoublePair p3(3.0, 3);
    rb.addNewElement(p3);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb.getNumberOfElements(), 2);
    ASSERT_EQ(rb.getWritePointer(), 1);
    ASSERT_EQ(rb.getDataVectorIndex(0), 1);
    ASSERT_EQ(rb.getDataVectorIndex(1), 0);
    ASSERT_EQ(rb.getDataVectorIndex(2), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), 1);
    ASSERT_EQ(rb.getRingBufferIndex(1), 0);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(0)), 0);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(1)), 1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE (rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[ 0]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[ 1]));
    ASSERT_TRUE (rb.isIndexOutOfBoundsElement(rb[ 2]));
    ASSERT_NE(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[1]);
    ASSERT_EQ(&rb.getOldestElement(), &rb[0]);
    ASSERT_NE(&rb.getNewestElement(), &rb[0]);
    ASSERT_NE(&rb.getOldestElement(), &rb[1]);
    ASSERT_EQ(rb.getOldestElement().value, p2.value);
    ASSERT_EQ(rb.getNewestElement().value, p3.value);
}
