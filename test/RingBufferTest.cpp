#include <gtest/gtest.h>
#include <RingBuffer.hpp>

using namespace libspeedwire;

// test index out of bounds methods
TEST(RingBufferTest, IndexOutOfBounds) {
    unsigned int el = 0;
    ASSERT_FALSE(RingBuffer<unsigned>::isIndexOutOfBoundsElement(el));
    
    const unsigned& outOfBounds = RingBuffer<unsigned>::getIndexOutOfBoundsElement();
    ASSERT_TRUE(RingBuffer<unsigned>::isIndexOutOfBoundsElement(outOfBounds));
}

// test zero capacity
TEST(RingBufferTest, Capacity0) {
    auto rb = RingBuffer<unsigned>(0);
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
    auto rb = RingBuffer<unsigned>(1);
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

    // buffer with one element 1
    rb.addNewElement(1);
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
    ASSERT_EQ(rb[0], 1);
    ASSERT_EQ(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(rb.getNewestElement(), 1);
    ASSERT_EQ(rb.at(0), rb[0]);

    // buffer with replaced element 2
    rb.addNewElement(2);
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
    ASSERT_EQ(rb[0], 2);
    ASSERT_EQ(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(rb.getNewestElement(), 2);
    ASSERT_EQ(rb.at(0), rb[0]);
}

// test capacity of two
TEST(RingBufferTest, Capacity2) {
    // empty buffer with cacpacity set in constructor
    auto rb = RingBuffer<unsigned>(2);
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

    // buffer with one element 1
    rb.addNewElement(1);
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
    ASSERT_EQ(rb[0], 1);
    ASSERT_EQ(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(rb.getNewestElement(), 1);
    ASSERT_EQ(rb.at(0), rb[0]);

    // buffer with two elements 1 and 2
    rb.addNewElement(2);
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
    ASSERT_EQ(rb.getRingBufferIndex(2), -1);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(0)), 0);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(1)), 1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE(rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[0]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[1]));
    ASSERT_EQ(rb[0], 1);
    ASSERT_EQ(rb[1], 2);
    ASSERT_NE(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_NE(&rb.getNewestElement(), &rb[0]);
    ASSERT_EQ(&rb.getOldestElement(), &rb[0]);
    ASSERT_EQ(&rb.getNewestElement(), &rb[1]);
    ASSERT_NE(&rb.getOldestElement(), &rb[1]);
    ASSERT_EQ(rb.getOldestElement(), 1);
    ASSERT_EQ(rb.getNewestElement(), 2);
    ASSERT_EQ(rb.at(0), rb[0]);
    ASSERT_EQ(rb.at(1), rb[1]);

    // buffer with two elements 2 and 3
    rb.addNewElement(3);
    ASSERT_EQ(rb.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb.getNumberOfElements(), 2);
    ASSERT_EQ(rb.getWritePointer(), 1);
    ASSERT_EQ(rb.getDataVectorIndex(-1), -1);
    ASSERT_EQ(rb.getDataVectorIndex(0), 1);
    ASSERT_EQ(rb.getDataVectorIndex(1), 0);
    ASSERT_EQ(rb.getDataVectorIndex(2), -1);
    ASSERT_EQ(rb.getRingBufferIndex(-1), -1);
    ASSERT_EQ(rb.getRingBufferIndex(0), 1);
    ASSERT_EQ(rb.getRingBufferIndex(1), 0);
    ASSERT_EQ(rb.getRingBufferIndex(2), -1);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(0)), 0);
    ASSERT_EQ(rb.getRingBufferIndex(rb.getDataVectorIndex(1)), 1);
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getNewestElement()));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb.getOldestElement()));
    ASSERT_TRUE (rb.isIndexOutOfBoundsElement(rb[-1]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[ 0]));
    ASSERT_FALSE(rb.isIndexOutOfBoundsElement(rb[ 1]));
    ASSERT_TRUE (rb.isIndexOutOfBoundsElement(rb[ 2]));
    ASSERT_EQ(rb[0], 2);
    ASSERT_EQ(rb[1], 3);
    ASSERT_NE(&rb.getNewestElement(), &rb.getOldestElement());
    ASSERT_EQ(&rb.getNewestElement(), &rb[1]);
    ASSERT_EQ(&rb.getOldestElement(), &rb[0]);
    ASSERT_NE(&rb.getNewestElement(), &rb[0]);
    ASSERT_NE(&rb.getOldestElement(), &rb[1]);
    ASSERT_EQ(rb.getOldestElement(), 2);
    ASSERT_EQ(rb.getNewestElement(), 3);
    ASSERT_EQ(rb.at(0), rb[0]);
    ASSERT_EQ(rb.at(1), rb[1]);
}
