#ifndef __LIBSPEEDWIRE_CALCULATEDVALUEPROCESSOR_HPP__
#define __LIBSPEEDWIRE_CALCULATEDVALUEPROCESSOR_HPP__

#include <cstdint>
#include <Consumer.hpp>
#include <Producer.hpp>
#include <ObisData.hpp>
#include <SpeedwireData.hpp>

namespace libspeedwire {

    /**
     *  Class CalculatedValueProcessor implements the calculation values derived from obis elements received from emeter
     *  packets and inverter reply packets.
     *
     *  The class is implemented as an ObisConsumer and SpeedwireConsumer. Values are passed on the obis_consumer and
     *  speedwire_consumer configured
     */
    class CalculatedValueProcessor : public ObisConsumer, SpeedwireConsumer {

    protected:

        ObisDataMap& obis_data_map;       //!< Reference to the data map, where all received obis values reside
        SpeedwireDataMap& speedwire_data_map;  //!< Reference to the data map, where all received inverter values reside
        Producer& producer;            //!< Reference to producer to receive the consumed and calculated values

    public:

        CalculatedValueProcessor(ObisDataMap& obis_map, SpeedwireDataMap& speedwire_map, Producer& producer);
        ~CalculatedValueProcessor(void);

        virtual void consume(const SpeedwireDevice& device, ObisData& element);
        virtual void consume(const SpeedwireDevice& device, SpeedwireData& element);

        virtual void endOfObisData(const SpeedwireDevice& device, const uint32_t time);
        virtual void endOfSpeedwireData(const SpeedwireDevice& device, const uint32_t time);
    };

}   // namespace libspeedwire

#endif
