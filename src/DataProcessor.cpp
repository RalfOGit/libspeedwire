#include <stdio.h>
#include <DataProcessor.hpp>
#include <Measurement.hpp>
#include <Producer.hpp>


DataProcessor::DataProcessor(const unsigned long averagingTime, Producer& producer) :
    producer(producer) {
    this->averagingTime = averagingTime;
    this->obis_remainder = 0;
    this->obis_currentTimestamp = 0;
    this->obis_currentTimestampIsValid = false;
    this->obis_averagingTimeReached = false;
    this->speedwire_remainder = 0;
    this->speedwire_currentTimestamp = 0;
    this->speedwire_currentTimestampIsValid = false;
    this->speedwire_averagingTimeReached = false;
}

DataProcessor::~DataProcessor(void) {}

void DataProcessor::consume(ObisData &element) {
    //element.print(stdout);
    MeasurementValue& measurement = element.measurementValue;

    // initialize current timestamp
    if (obis_currentTimestampIsValid == false) {
        obis_currentTimestampIsValid = true;
    }
    // check if this is the first measurement of a new measurement block
    else if (measurement.timer != obis_currentTimestamp) {
        obis_remainder += measurement.elapsed;
        obis_averagingTimeReached = (obis_remainder >= averagingTime);
        //printf("obis_averagingTimeReached %d\n", obis_averagingTimeReached);
        if  (obis_averagingTimeReached == true) {
            obis_remainder %= averagingTime;
        }
    }
    obis_currentTimestamp = measurement.timer;

    // if less than the defined averaging time has elapsed, sum up values
    if (obis_averagingTimeReached == false) {
        if (isInstantaneous(element.measurementType.quantity) == true) {
            measurement.sumValue += measurement.value;
            measurement.counter++;
        }
    }
    // if the averaging time has elapsed, prepare a field for the influxdb consumer
    else {
        double value;
        if (isInstantaneous(element.measurementType.quantity) == true) {
            if (measurement.counter > 0) {
                value = measurement.sumValue / measurement.counter;
            } else { 
                value = measurement.value;
            }
            measurement.sumValue = 0;
            measurement.counter = 0;
        } else {
            value = measurement.value;
        }
        producer.produce("meter", element.measurementType, element.line, value);
    }
}


void DataProcessor::consume(SpeedwireData& element) {
    //element.print(stdout); fprintf(stdout, "speedwire_currentTimestamp %ld\n", speedwire_currentTimestamp);
    MeasurementValue& measurement = element.measurementValue;
    // initialize current timestamp
    if (speedwire_currentTimestampIsValid == false) {
        speedwire_currentTimestampIsValid = true;
    }
    // check if this is the first measurement of a new measurement block
    else if ((int32_t)(measurement.timer - speedwire_currentTimestamp) > 2 || (int32_t)(speedwire_currentTimestamp - measurement.timer) > 2) {   // inverter time may increase during a query block
        speedwire_remainder += measurement.elapsed;
        speedwire_averagingTimeReached = (speedwire_remainder >= (averagingTime / 1000));   // inverter timestamps are in seconds
        if (speedwire_averagingTimeReached == true) {
            speedwire_remainder %= (averagingTime / 1000);
        }
    }
    speedwire_currentTimestamp = measurement.timer;

    // if less than the defined averaging time has elapsed, sum up values
    if (speedwire_averagingTimeReached == false) {
        if (isInstantaneous(element.measurementType.quantity) == true) {
            measurement.sumValue += measurement.value;
            measurement.counter++;
        }
    }
    // if the averaging time has elapsed, prepare a field for the influxdb consumer
    else {
        double value;
        if (isInstantaneous(element.measurementType.quantity) == true) {
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
        producer.produce("inverter", element.measurementType, element.wire, value);
    }
}
