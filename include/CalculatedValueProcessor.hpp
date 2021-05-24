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

        virtual void consume(const uint32_t serial_number, ObisData& element);
        virtual void consume(const uint32_t serial_number, SpeedwireData& element);

        virtual void endOfObisData(const uint32_t serial_number, const uint32_t time);
        virtual void endOfSpeedwireData(const uint32_t serial_number, const uint32_t time);
    };

}   // namespace libspeedwire

#endif
