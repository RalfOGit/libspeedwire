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

// test number of elements methods
TEST(RingBufferTest, NumberOfElements) {
    RingBuffer<int> rb0(0);
    RingBuffer<int> rb1(1);
    RingBuffer<int> rb2(2);
    RingBuffer<int> rb3(3);
    int value = 0;

    // empty buffer with cacpacity set in constructor
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 0);
    ASSERT_EQ(rb3.getNumberOfElements(), 0);

    // empty buffer with cacpacity set explicitly
    rb0.setMaximumNumberOfElements(0);
    rb1.setMaximumNumberOfElements(1);
    rb2.setMaximumNumberOfElements(2);
    rb3.setMaximumNumberOfElements(3);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 0);
    ASSERT_EQ(rb3.getNumberOfElements(), 0);

    // add one element
    rb0.addNewElement(value);
    rb1.addNewElement(value);
    rb2.addNewElement(value);
    rb3.addNewElement(value);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 1);
    ASSERT_EQ(rb3.getNumberOfElements(), 1);

    // add second element
    rb0.addNewElement(value);
    rb1.addNewElement(value);
    rb2.addNewElement(value);
    rb3.addNewElement(value);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 2);
    ASSERT_EQ(rb3.getNumberOfElements(), 2);

    // add third element
    rb0.addNewElement(value);
    rb1.addNewElement(value);
    rb2.addNewElement(value);
    rb3.addNewElement(value);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 2);
    ASSERT_EQ(rb3.getNumberOfElements(), 3);

    // add fourth element
    rb0.addNewElement(value);
    rb1.addNewElement(value);
    rb2.addNewElement(value);
    rb3.addNewElement(value);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 2);
    ASSERT_EQ(rb3.getNumberOfElements(), 3);
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

// test clear and remove methods
TEST(RingBufferTest, RemoveElements) {
    RingBuffer<int> rb0(0);
    RingBuffer<int> rb1(1);
    RingBuffer<int> rb2(2);
    RingBuffer<int> rb3(3);

    // empty buffer cleared
    rb0.clear();
    rb1.clear();
    rb2.clear();
    rb3.clear();
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 0);
    ASSERT_EQ(rb3.getNumberOfElements(), 0);

    // empty buffer elements removed
    ASSERT_EQ(rb0.removeElements(0, 1), 0);
    ASSERT_EQ(rb1.removeElements(0, 1), 0);
    ASSERT_EQ(rb2.removeElements(0, 1), 0);
    ASSERT_EQ(rb3.removeElements(0, 1), 0);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 0);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 0);
    ASSERT_EQ(rb3.getNumberOfElements(), 0);

    // add one element, then clear
    rb0.addNewElement(0);
    rb1.addNewElement(0);
    rb2.addNewElement(0);
    rb3.addNewElement(0);
    rb0.clear();
    rb1.clear();
    rb2.clear();
    rb3.clear();
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 0);
    ASSERT_EQ(rb3.getNumberOfElements(), 0);

    // add one element, then remove
    rb0.addNewElement(1);
    rb1.addNewElement(1);
    rb2.addNewElement(1);
    rb3.addNewElement(1);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 1);
    ASSERT_EQ(rb3.getNumberOfElements(), 1);
    ASSERT_EQ(rb0.removeElements(0, 1), 1);
    ASSERT_EQ(rb1.removeElements(0, 1), 1);
    ASSERT_EQ(rb2.removeElements(0, 1), 1);
    ASSERT_EQ(rb3.removeElements(0, 1), 1);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 0);
    ASSERT_EQ(rb3.getNumberOfElements(), 0);

    // add two elements, then remove first
    rb0.addNewElement(1); rb0.addNewElement(2);
    rb1.addNewElement(1); rb1.addNewElement(2);
    rb2.addNewElement(1); rb2.addNewElement(2);
    rb3.addNewElement(1); rb3.addNewElement(2);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 2);
    ASSERT_EQ(rb3.getNumberOfElements(), 2);
    ASSERT_EQ(rb0.removeElements(0, 1), 1);
    ASSERT_EQ(rb1.removeElements(0, 1), 1);
    ASSERT_EQ(rb2.removeElements(0, 1), 1);
    ASSERT_EQ(rb3.removeElements(0, 1), 1);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 1);
    ASSERT_EQ(rb3.getNumberOfElements(), 1);
    ASSERT_EQ(rb2[0], 2);
    ASSERT_EQ(rb3[0], 2);

    // add two elements, then remove second
    rb0.addNewElement(3); rb0.addNewElement(4);
    rb1.addNewElement(3); rb1.addNewElement(4);
    rb2.addNewElement(3); rb2.addNewElement(4);
    rb3.addNewElement(3); rb3.addNewElement(4);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 2);
    ASSERT_EQ(rb3.getNumberOfElements(), 3);
    ASSERT_EQ(rb0.removeElements(1, 1), 0);     // there is no element at index 1, no effect
    ASSERT_EQ(rb1.removeElements(1, 1), 0);     // there is no element at index 1, no effect
    ASSERT_EQ(rb2.removeElements(1, 1), 1);
    ASSERT_EQ(rb3.removeElements(1, 1), 1);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 1);
    ASSERT_EQ(rb3.getNumberOfElements(), 2);
    ASSERT_EQ(rb0[0], 4);
    ASSERT_EQ(rb1[0], 4);
    ASSERT_EQ(rb2[0], 3);
    ASSERT_EQ(rb3[0], 2);
    ASSERT_EQ(rb3[1], 4);

    // add third element, then remove last element
    rb0.addNewElement(5);
    rb1.addNewElement(5);
    rb2.addNewElement(5);
    rb3.addNewElement(5);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 1);
    ASSERT_EQ(rb1.getNumberOfElements(), 1);
    ASSERT_EQ(rb2.getNumberOfElements(), 2);
    ASSERT_EQ(rb3.getNumberOfElements(), 3);
    ASSERT_EQ(rb0.removeElements(rb0.getNumberOfElements() - 1, 1), 1);
    ASSERT_EQ(rb1.removeElements(rb1.getNumberOfElements() - 1, 1), 1);
    ASSERT_EQ(rb2.removeElements(rb2.getNumberOfElements() - 1, 1), 1);
    ASSERT_EQ(rb3.removeElements(rb3.getNumberOfElements() - 1, 1), 1);
    ASSERT_EQ(rb0.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb1.getMaximumNumberOfElements(), 1);
    ASSERT_EQ(rb2.getMaximumNumberOfElements(), 2);
    ASSERT_EQ(rb3.getMaximumNumberOfElements(), 3);
    ASSERT_EQ(rb0.getNumberOfElements(), 0);
    ASSERT_EQ(rb1.getNumberOfElements(), 0);
    ASSERT_EQ(rb2.getNumberOfElements(), 1);
    ASSERT_EQ(rb3.getNumberOfElements(), 2);
}
