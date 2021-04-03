#include <Measurement.hpp>


MeasurementType::MeasurementType(const Direction direction, const Type type, const Quantity quantity, 
                                 const std::string &unit, const unsigned long divisor) {
    this->direction = direction;
    this->type = type;
    this->quantity = quantity;
    this->unit = unit;
    this->divisor = divisor;
    this->instaneous = isInstantaneous(quantity);
    std::string direction_string = toString(direction);
    std::string type_string      = toString(type);
    std::string quantity_string  = toString(quantity);
    if (direction_string.length() > 0) direction_string.append("_");
    if (type_string.length() > 0 && quantity_string.length() > 0) type_string.append("_");
    this->name = direction_string.append(type_string).append(quantity_string);
}

std::string MeasurementType::getFullName(const Line line) const {
    if (line != Line::TOTAL && line != Line::NO_LINE) {
        return std::string(name).append("_").append(toString(line));
    }
    return name;
}


/*******************************/

MeasurementValue::MeasurementValue(void) {
    value = 0.0;
    value_string = "";
    timer = 0;
    elapsed = 0;
    sumValue = 0;
    counter = 0;
    initial = true;
}

void MeasurementValue::setValue(int32_t raw_value, unsigned long divisor) {
    value = (double)raw_value / (double)divisor;
}

void MeasurementValue::setValue(uint32_t raw_value, unsigned long divisor) {
    value = (double)raw_value / (double)divisor;
}

void MeasurementValue::setValue(uint64_t raw_value, unsigned long divisor) {
    value = (double)raw_value / (double)divisor;
}

void MeasurementValue::setValue(const std::string& raw_value) {
    value_string = raw_value;
}

void MeasurementValue::setTimer(uint32_t time) {
    if (initial) {
        initial = false;
        timer = time;
        elapsed = 1000;
    } else {
        elapsed = time - timer;
        timer = time;
    }
}

/*******************************/

std::string toString(const Direction direction) {
    if (direction == Direction::POSITIVE) return "positive";
    if (direction == Direction::NEGATIVE) return "negative";
    if (direction == Direction::SIGNED)   return "signed";
    if (direction == Direction::NO_DIRECTION) return "";
    return "undefined direction";
}

std::string toString(const Line line) {
    switch (line) {
        case Line::TOTAL:     return "total";
        case Line::L1:        return "l1";
        case Line::L2:        return "l2";
        case Line::L3:        return "l3";
        case Line::MPP_TOTAL: return "mpp_total";
        case Line::MPP1:      return "mpp1";
        case Line::MPP2:      return "mpp2";
        case Line::LOSS_TOTAL:return "loss_total";
        case Line::DEVICE_OK: return "device_ok";
        case Line::RELAY_ON:  return "relay_on";
        case Line::NO_LINE:   return "";
    }
    return "undefined line";
}

std::string toString(const Quantity quantity) {
    switch (quantity) {
        case Quantity::POWER:        return "power";
        case Quantity::ENERGY:       return "energy";
        case Quantity::POWER_FACTOR: return "power_factor";
        case Quantity::VOLTAGE:      return "voltage";
        case Quantity::CURRENT:      return "current";
        case Quantity::STATUS:       return "status";
        case Quantity::EFFICIENCY:   return "efficiency";
        case Quantity::NO_QUANTITY:  return "";
    }
    return "undefined quantity";
}

bool isInstantaneous(const Quantity quantity) {
    return (quantity != Quantity::ENERGY);
}

std::string toString(const Type type) {
    switch(type) {
        case Type::ACTIVE:      return "active";
        case Type::REACTIVE:    return "reactive";
        case Type::APPARENT:    return "apparent";
        case Type::VERSION:     return "version";
        case Type::END_OF_DATA: return "end of data";
        case Type::NO_TYPE:     return "";
    }
    return "undefined type";
}
