#ifndef __LIBSPEEDWIRE_RINGBUFFER_HPP__
#define __LIBSPEEDWIRE_RINGBUFFER_HPP__

#include <vector>

namespace libspeedwire {

    /**
     *  Class encapsulating a ring buffer for elements of type T.
     */
    template<class T> class RingBuffer {
    public:
        std::vector<T>  data_vector;    //!< Array of ring buffer elements
        size_t          write_pointer;  //!< Write pointer pointing to the next element to write to

        /**
         * Constructor.
         * @param capacity Maximum number of ring buffer elements
         */
        RingBuffer(const int capacity) :
            data_vector(capacity) {
            clear();
        }

        /**
         *  Delete all elements from the ring buffer.
         */
        void clear(void) {
            data_vector.clear();
            write_pointer = 0;
        }

        /**
         *  Get maximum number of elements that can be stored in the ring buffer.
         *  @return the maximum number
         */
        size_t getMaximumNumberOfElements(void) const {
            return data_vector.capacity();
        }

        /**
         *  Set maximum number of elements that can be stored in the ring buffer.
         *  This will clear any elements before resizing the ring buffer.
         *  @param new_capacity the maximum number
         */
        void setMaximumNumberOfElements(const size_t new_capacity) {
            clear();
            data_vector.reserve(new_capacity);
        }

        /**
         *  Get number of elements that are currently stored in the ring buffer.
         *  @return the number
         */
        size_t getNumberOfElements(void) const {
            return data_vector.size();
        }

        /**
         *  Add a new element to the ring buffer. If the buffer is full, the oldest element is replaced.
         *  @param value the element value
         */
        void addNewElement(const T &value) {
            if (data_vector.size() > write_pointer) {
                data_vector[write_pointer] = value;
            }
            else {
                data_vector.push_back(value);
            }
            if (++write_pointer >= data_vector.capacity()) {
                write_pointer = 0;
            }
        }

        /**
         *  Get a reference to the element at the given ring buffer index position.
         *  @param i ring buffer index - i = 0 gets the oldest element, i = (getNumberOfElements()-1) gets the newest element.
         *  @return reference to the element at ring buffer index; if the index is out of bounds, reference getIndexOutOfBoundsElement() is returned. 
         */
        const T& operator[](const size_t i) const {
            const size_t index = getDataVectorIndex(i);
            if (index != (size_t)-1) {
                return data_vector[index];
            }
            return getIndexOutOfBoundsElement();
        }

        /**
         *  Get a reference to the newest element in the ring buffer.
         *  @return reference to the newest element; if the ring buffer is empty, reference getIndexOutOfBoundsElement() is returned.
         */
        const T& getNewestElement(void) const {
            return operator[](data_vector.size() - 1);
        }

        /**
         *  Get a reference to the oldest element in the ring buffer.
         *  @return reference to the oldest element; if the ring buffer is empty, reference getIndexOutOfBoundsElement() is returned.
         */
        const T& getOldestElement(void) const {
            return operator[](0);
        }

        //
        //  Methods exposing the internal representation
        //

        /**
         *  Get a reference to the underlying element array.
         *  @return reference to array
         */
        const std::vector<T>& getDataVector(void) const {
            return data_vector;
        }

        /**
         *  Get the write pointer indexing the internal element array.
         *  The write pointer points to the next write position in the internal element array.
         *  @return write pointer index
         */
        size_t getWritePointer(void) const {
            return write_pointer;
        }

        /**
         *  Get the data vector index corresponding to the given ring buffer index.
         *  @param ring_buffer_index the ring buffer index.
         *  @return the data vector index, (size_t)-1 in case of index out of bounds condition.
         */
        size_t getDataVectorIndex(const size_t ring_buffer_index) const {
            const size_t size = data_vector.size();
            if (ring_buffer_index < size) {
                size_t index = write_pointer + ring_buffer_index;
                if (index >= size) {
                    index -= size;
                }
                return index;
            }
            return (size_t)-1;
        }

        /**
         *  Get the ring buffer index corresponding to the given data vector index.
         *  @param data_vector_index the data vector index.
         *  @return the ring buffer index, (size_t)-1 in case of index out of bounds condition.
         */
        size_t getRingBufferIndex(const size_t data_vector_index) const {
            const size_t size = data_vector.size();
            if (data_vector_index < size) {
                size_t index = data_vector_index - write_pointer;
                if (index >= size) {    // modulo arithmetic!
                    index += size;
                }
                return index;
            }
            return (size_t)-1;
        }

        //
        //  Methods to handle index out of bounds conditions
        //

        /**
         *  Get a reference to a static element that is used to indicate index out of bounds conditions.
         *  @return reference to the index out of bound element.
         */
        static const T& getIndexOutOfBoundsElement(void) {
            static const T el;
            return el;
        }

        /**
         *  Check if the given element reference is identical to the static index out of bounds element.
         *  @return true or false
         */
        static bool isIndexOutOfBoundsElement(const T& element) {
            const T& indexOutOfBoundsElement = getIndexOutOfBoundsElement();
            return (&element == &indexOutOfBoundsElement);
        }

    };

}   // namespace libspeedwire

#endif
