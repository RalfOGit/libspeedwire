#include <MeasurementType.hpp>
#include <MeasurementValues.hpp>
using namespace libspeedwire;

TimestampDoublePair TimestampDoublePair::defaultPair = TimestampDoublePair(0.0, 0);

/**
* Constructor.
* @param direction Direction of the energy flow
* @param type  Type of the measurement
* @param quantity Physical quantity of the measurement
* @param unit Measurement unit after applying the divisor, e.g. W, kWh
* @param divisor Divide value by divisor to obtain floating point measurements in the given unit
*/
MeasurementType::MeasurementType(const Direction direction, const Type type, const Quantity quantity,
    const std::string& unit, const unsigned long divisor) {
    this->direction = direction;
    this->type = type;
    this->quantity = quantity;
    this->unit = unit;
    this->divisor = divisor;
    this->instaneous = isInstantaneous(quantity);
    std::string direction_string = toString(direction);
    std::string type_string = toString(type);
    std::string quantity_string = toString(quantity);
    if (direction_string.length() > 0) direction_string.append("_");
    if (type_string.length() > 0 && quantity_string.length() > 0) type_string.append("_");
    this->name = direction_string.append(type_string).append(quantity_string);
}

//! Get full name of this instance; this is the name concatenated with the Wire string (if wire is a single wire)
std::string MeasurementType::getFullName(const Wire wire) const {
    if (wire != Wire::TOTAL && wire != Wire::NO_WIRE) {
        return std::string(name).append("_").append(toString(wire));
    }
    return name;
}

std::string libspeedwire::toString(const Direction direction) {
    if (direction == Direction::POSITIVE) return "positive";
    if (direction == Direction::NEGATIVE) return "negative";
    if (direction == Direction::SIGNED)   return "signed";
    if (direction == Direction::NO_DIRECTION) return "";
    return "undefined direction";
}

std::string libspeedwire::toString(const Wire wire) {
    switch (wire) {
    case Wire::TOTAL:            return "total";
    case Wire::L1:               return "l1";
    case Wire::L2:               return "l2";
    case Wire::L3:               return "l3";
    case Wire::MPP_TOTAL:        return "mpp_total";
    case Wire::MPP1:             return "mpp1";
    case Wire::MPP2:             return "mpp2";
    case Wire::LOSS_TOTAL:       return "loss_total";
    case Wire::DEVICE_OK:        return "device_ok";
    case Wire::RELAY_ON:         return "relay_on";
    case Wire::FEED_IN:          return "feed_in";
    case Wire::SELF_CONSUMPTION: return "self_consumption";
    case Wire::NO_WIRE:          return "";
    }
    return "undefined wire";
}

std::string libspeedwire::toString(const Quantity quantity) {
    switch (quantity) {
    case Quantity::POWER:        return "power";
    case Quantity::ENERGY:       return "energy";
    case Quantity::POWER_FACTOR: return "power_factor";
    case Quantity::FREQUENCY:    return "frequency";
    case Quantity::VOLTAGE:      return "voltage";
    case Quantity::CURRENT:      return "current";
    case Quantity::STATUS:       return "status";
    case Quantity::EFFICIENCY:   return "efficiency";
    case Quantity::CURRENCY:     return "currency";
    case Quantity::NO_QUANTITY:  return "";
    }
    return "undefined quantity";
}

bool libspeedwire::isInstantaneous(const Quantity quantity) {
    return (quantity != Quantity::ENERGY);
}

std::string libspeedwire::toString(const Type type) {
    switch (type) {
    case Type::ACTIVE:      return "active";
    case Type::REACTIVE:    return "reactive";
    case Type::APPARENT:    return "apparent";
    case Type::VERSION:     return "version";
    case Type::END_OF_DATA: return "end of data";
    case Type::NO_TYPE:     return "";
    }
    return "undefined type";
}
