#include <stdio.h>
#include <AveragingProcessor.hpp>
#include <Measurement.hpp>
#include <Producer.hpp>


/**
 * Constructor of the DataProcesor instance.
 * @param averagingTime Constant averaging time for data received from any of the emeter or inverter data inputs.
 * @param producer Reference to the data producer that will receive the data after processing it in this instance.
 */
AveragingProcessor::AveragingProcessor(const unsigned long averaging_time, Producer& producer) :
    averagingTime(averaging_time),
    producer(producer) {}


/**
 * Destructor.
 */
AveragingProcessor::~AveragingProcessor(void) {}


/**
 * Initialize/add a block of state keeping variables for averaging measurement value of the given device.
 * @param device The device identifier.
 * @return Index of the newly initialized/added variable block in vector states.
 */
int AveragingProcessor::initializeState(const uint32_t serial_number, const DeviceType& device_type) {
    AveragingState device_state;
    device_state.serialNumber            = serial_number;
    device_state.deviceType              = device_type;
    device_state.remainder               = 0;
    device_state.currentTimestamp        = 0;
    device_state.currentTimestampIsValid = false;
    device_state.averagingTimeReached    = false;
    states.push_back(device_state);
    return (int)states.size() - 1;
}


/**
 * Find block of state keeping variables for averaging measurement value of the given device.
 * @param device The device identifier.
 * @return Index of the variable block in vector states, or -1 if there is none.
 */
int AveragingProcessor::findStateIndex(const uint32_t serial_number) {
    for (int i = 0; i < states.size(); i++) {
        if (states[i].serialNumber == serial_number) {
            return i;
        }
    }
    return -1;
}


/**
 * Internal implementation for temporal averaging of emeter obis values or inverter values.
 * @param device The device identifier.
 * @param type The measurement type.
 * @param measurement The measurement value.
 * @param wire The wire identifier.
 */
void AveragingProcessor::consume(const uint32_t serial_number, const DeviceType& device_type, MeasurementType& type, MeasurementValue& measurement, Wire& wire) {

    // find device
    int index = findStateIndex(serial_number);
    if (index < 0) {
        index = initializeState(serial_number, device_type);
    }
    AveragingState& state = states[index];

    // initialize current timestamp
    if (state.currentTimestampIsValid == false) {
        state.currentTimestampIsValid = true;
    }
    // check if this is the first measurement of a new measurement block
    else if (measurement.timer != state.currentTimestamp) {
        state.remainder += measurement.elapsed;
        switch (state.deviceType) {
        case DeviceType::EMETER:    state.averagingTimeReached = (state.remainder >= averagingTime); break;
        case DeviceType::INVERTER:
        default:            state.averagingTimeReached = (state.remainder >= (averagingTime / 1000)); break;
        }
        //printf("averagingTimeReached %d\n", state.averagingTimeReached);
        if (state.averagingTimeReached == true) {
            state.remainder %= averagingTime;
        }
    }
    state.currentTimestamp = measurement.timer;

    // if less than the defined averaging time has elapsed, sum up values
    if (state.averagingTimeReached == false) {
        if (isInstantaneous(type.quantity) == true) {
            measurement.sumValue += measurement.value;
            measurement.counter++;
        }
    }
    // if the averaging time has elapsed, prepare a field for the influxdb consumer
    else {
        double value;
        if (isInstantaneous(type.quantity) == true) {
            if (measurement.counter > 0) {
                value = measurement.sumValue / measurement.counter;
            }
            else {
                value = measurement.value;
            }
            measurement.sumValue = 0;
            measurement.counter = 0;
        }
        else {
            value = measurement.value;
        }
        producer.produce(serial_number, type, wire, value);
    }
}


/**
 * Callback to consume the given obis data element - implements the temporal averaging of obis values.
 * @param element A reference to an ObisData instance, holding output data of the ObisFilter.
 */
void AveragingProcessor::consume(const uint32_t serial_number, ObisData &element) {
    //element.print(stdout);
    consume(serial_number, DeviceType::EMETER, element.measurementType, element.measurementValue, element.wire);
}


/**
 * Callback to consume the given inverter reply data element - implements the temporal averaging of inverter values.
 * @param element A reference to an SpeedwireData instance.
 */
void AveragingProcessor::consume(const uint32_t serial_number, SpeedwireData& element) {
    //element.print(stdout); fprintf(stdout, "speedwire_currentTimestamp %ld\n", speedwire_currentTimestamp);
    consume(serial_number, DeviceType::INVERTER, element.measurementType, element.measurementValue, element.wire);
}
