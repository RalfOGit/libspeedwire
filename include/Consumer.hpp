#ifndef __LIBSPEEDWIRE_CONSUMER_HPP__
#define __LIBSPEEDWIRE_CONSUMER_HPP__

#include <ObisData.hpp>
#include <SpeedwireData.hpp>
#include <SpeedwireDevice.hpp>

namespace libspeedwire {

    /**
     *  Interface to be implemented by any obis consumer; that is any consumer that likes to receive data from class ObisFilter.
     */
    class ObisConsumer {
    public:
        /** Virtual destructor. */
        virtual ~ObisConsumer(void) {}

        /**
         * Callback to produce the given obis data to the next stage in the processing pipeline.
         * @param device The originating inverter device.
         * @param element A reference to a received ObisData instance, holding output data of the ObisFilter.
         */
        virtual void consume(const SpeedwireDevice& device, ObisData& element) = 0;

        /**
         * Callboack to notify that the last obis data in the emeter packet has been processed.
         * @param device The originating inverter device.
         * @param timestamp The timestamp associated with the just finished emeter packet.
         */
        virtual void endOfObisData(const SpeedwireDevice& device, const uint32_t timestamp) {}
    };


    /**
     *  Interface to be implemented by the consumer of speedwire inverter reply data.
     */
    class SpeedwireConsumer {
    public:
        /** Virtual destructor */
        virtual ~SpeedwireConsumer(void) {}

        /**
         * Consume a speedwire reply data element
         * @param device The originating inverter device.
         * @param element A reference to a received SpeedwireData instance.
         */
        virtual void consume(const SpeedwireDevice& device, SpeedwireData& element) = 0;

        /**
         * Callboack to notify that the last data in the inverter packet has been processed.
         * @param device The originating inverter device.
         * @param timestamp The timestamp associated with the just finished inverter packet.
         */
        virtual void endOfSpeedwireData(const SpeedwireDevice&device, const uint32_t timestamp) {}
    };

}   // namespace libspeedwire

#endif
