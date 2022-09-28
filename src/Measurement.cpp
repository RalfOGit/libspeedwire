#include <Measurement.hpp>
using namespace libspeedwire;


//! Constructor
MeasurementValue::MeasurementValue(void) {
    value = 0.0;
    value_string = "";
    timer = 0;
    elapsed = 0;
    sumValue = 0;
    counter = 0;
    lastValue = 0.0;
    initial = true;
}

//! Set the value of this instance to the given raw_value divided by the divisor
void MeasurementValue::setValue(int32_t raw_value, unsigned long divisor) {
    value = (double)raw_value / (double)divisor;
}

//! Set the value of this instance to the given raw_value divided by the divisor
void MeasurementValue::setValue(uint32_t raw_value, unsigned long divisor) {
    value = (double)raw_value / (double)divisor;
}

//! Set the value of this instance to the given raw_value divided by the divisor
void MeasurementValue::setValue(uint64_t raw_value, unsigned long divisor) {
    value = (double)raw_value / (double)divisor;
}

//! Set the value of this instance to the given raw_value
void MeasurementValue::setValue(const std::string& raw_value) {
    value_string = raw_value;
}

//! Set the timer of this instance to the given value
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
