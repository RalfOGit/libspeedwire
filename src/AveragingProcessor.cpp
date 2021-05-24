#include <stdio.h>
#include <AveragingProcessor.hpp>
#include <Measurement.hpp>
#include <Producer.hpp>
using namespace libspeedwire;


/**
 * Constructor of the AveragingProcessor instance.
 * @param averaging_time Constant averaging time for data received from any of the emeter or inverter data inputs.
 */
AveragingProcessor::AveragingProcessor(const unsigned long averaging_time) :
    averagingTime(averaging_time) {}


/**
 * Destructor.
 */
AveragingProcessor::~AveragingProcessor(void) {}


/**
 * Initialize/add a block of state keeping variables for averaging measurement value of the given device.
 * @param serial_number The serial number of the device.
 * @param device_type The device identifier.
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
 * @param serial_number The serial number of the device.
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
 * Add an obis consumer to receive the result of the AveragingProcessor.
 * @param obis_consumer Reference to the ObisConsumer.
 */
void AveragingProcessor::addConsumer(ObisConsumer& obis_consumer) {
    obisConsumerTable.push_back(&obis_consumer);
}


/**
 * Add an speedwire consumer to receive the result of the AveragingProcessor.
 * @param speedwire_consumer Reference to the SpeedwireConsumer.
 */
void AveragingProcessor::addConsumer(SpeedwireConsumer& speedwire_consumer) {
    speedwireConsumerTable.push_back(&speedwire_consumer);
}


/**
 * Internal implementation for temporal averaging of emeter obis values or inverter values.
 * @param serial_number The serial number of the device.
 * @param device_type The device type.
 * @param type The measurement type.
 * @param measurement The measurement value.
 * @return true if the averaging time perios has elapsed, false otherwise.
 */
bool AveragingProcessor::process(const uint32_t serial_number, const DeviceType& device_type, MeasurementType& type, MeasurementValue& measurement) {

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
        default:                    state.averagingTimeReached = (state.remainder >= (averagingTime / 1000)); break;
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
        if (isInstantaneous(type.quantity) == true) {
            if (measurement.counter > 0) {
                measurement.value = measurement.sumValue / measurement.counter;
            }
            measurement.sumValue = 0;
            measurement.counter = 0;
        }
        return true;
    }
    return false;
}


/**
 * Callback to consume the given obis data element - implements the temporal averaging of obis values.
 * @param serial_number The serial number of the originating emeter device.
 * @param element A reference to an ObisData instance, holding output data of the ObisFilter.
 */
void AveragingProcessor::consume(const uint32_t serial_number, ObisData &element) {
    //element.print(stdout);
    if (process(serial_number, DeviceType::EMETER, element.measurementType, element.measurementValue) == true) {
        for (int i = 0; i < obisConsumerTable.size(); ++i) {
            obisConsumerTable[i]->consume(serial_number, element);
        }
    }
}


/**
 * Callback to consume the given inverter reply data element - implements the temporal averaging of inverter values.
 * @param serial_number The serial number of the originating inverter device.
 * @param element A reference to an SpeedwireData instance.
 */
void AveragingProcessor::consume(const uint32_t serial_number, SpeedwireData& element) {
    //element.print(stdout); fprintf(stdout, "speedwire_currentTimestamp %ld\n", speedwire_currentTimestamp);
    if (process(serial_number, DeviceType::INVERTER, element.measurementType, element.measurementValue) == true) {
        for (int i = 0; i < speedwireConsumerTable.size(); ++i) {
            speedwireConsumerTable[i]->consume(serial_number, element);
        }
    }
}


/**
 * Callback to notify that the last obis data in the emeter packet has been processed.
 * @param serial_number The serial number of the originating emeter device.
 * @param time The timestamp associated with the just finished emeter packet.
 */
void AveragingProcessor::endOfObisData(const uint32_t serial_number, const uint32_t time) {
    // if averaging time has been reached, signal end of obis data
    int index = findStateIndex(serial_number);
    if (index >= 0 && states[index].averagingTimeReached == true) {
        for (int i = 0; i < obisConsumerTable.size(); ++i) {
            obisConsumerTable[i]->endOfObisData(serial_number, time);
        }
    }
}


/**
 * Callback to notify that the last obis data in the inverter packet has been processed.
 * @param serial_number The serial number of the originating inverter device.
 * @param time The timestamp associated with the just finished inverter packet.
 */
void AveragingProcessor::endOfSpeedwireData(const uint32_t serial_number, const uint32_t time) {
    // if averaging time has been reached, signal end of obis data
    int index = findStateIndex(serial_number);
    if (index >= 0 && states[index].averagingTimeReached == true) {
        for (int i = 0; i < speedwireConsumerTable.size(); ++i) {
            speedwireConsumerTable[i]->endOfSpeedwireData(serial_number, time);
        }
    }
}
