#ifndef __LIBSPEEDWIRE_MEASUREMENT_HPP__
#define __LIBSPEEDWIRE_MEASUREMENT_HPP__

#include <string>
#include <MeasurementType.hpp>
#include <MeasurementValues.hpp>

namespace libspeedwire {

    /**
     *  Class holding measurement values together with their corresponding measurement type definition.
     */
    class Measurement {
    public:
        MeasurementType   measurementType;
        MeasurementValues measurementValues;
        Wire              wire;
        std::string       description;

        /**
         *  Constructor.
         *  @param mType   measurement type
         *  @param wire_   measurement wire
         */
        Measurement(const MeasurementType& mType, const Wire& mWire) :
            measurementType(mType),
            wire(mWire),
            description(mType.getFullName(mWire)),
            measurementValues(0) {
        }
    };

}   // namespace libspeedwire

#endif
