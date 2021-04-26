#include <CalculatedValueProcessor.hpp>

CalculatedValueProcessor::CalculatedValueProcessor(ObisDataMap& obis_map, SpeedwireDataMap& speedwire_map, Producer& _producer) :
    obis_data_map(obis_map),
    speedwire_data_map(speedwire_map),
    producer(_producer) {
}

CalculatedValueProcessor::~CalculatedValueProcessor(void) { }


/**
 * Callback to produce the given obis data to the next stage in the processing pipeline.
 * @param serial The serial number of the originating emeter device.
 * @param element A reference to a received ObisData instance, holding output data of the ObisFilter.
 */
void CalculatedValueProcessor::consume(const uint32_t serial_number, ObisData& element) {
    producer.produce(serial_number, element.measurementType, element.wire, element.measurementValue.value);
}


/**
 * Consume a speedwire reply data element
 * @param serial The serial number of the originating inverter device.
 * @param element A reference to a received SpeedwireData instance.
 */
void CalculatedValueProcessor::consume(const uint32_t serial_number, SpeedwireData& element) {
    producer.produce(serial_number, element.measurementType, element.wire, element.measurementValue.value);
}


/**
 * Callback to notify that the last obis data in the emeter packet has been processed.
 * @param serial The serial number of the originating emeter device.
 * @param time The timestamp associated with the just finished emeter packet.
 */
void CalculatedValueProcessor::endOfObisData(const uint32_t serial_number, const uint32_t time) {
    ObisDataMap::const_iterator pos, neg, end = obis_data_map.end();
    ObisDataMap::iterator sig;

    // calculate signed power L1
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL1.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL1.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL1.toKey())) != end) {
        sig->second.measurementValue.value = pos->second.measurementValue.value - neg->second.measurementValue.value;
        sig->second.measurementValue.timer = time;
        producer.produce(serial_number, ObisData::SignedActivePowerL1.measurementType, ObisData::SignedActivePowerL1.wire, sig->second.measurementValue.value);
    }

    // calculate signed power L2
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL2.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL2.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL2.toKey())) != end) {
        sig->second.measurementValue.value = pos->second.measurementValue.value - neg->second.measurementValue.value;
        sig->second.measurementValue.timer = time;
        producer.produce(serial_number, ObisData::SignedActivePowerL2.measurementType, ObisData::SignedActivePowerL2.wire, sig->second.measurementValue.value);
    }

    // calculate signed power L3
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL3.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL3.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL3.toKey())) != end) {
        sig->second.measurementValue.value = pos->second.measurementValue.value - neg->second.measurementValue.value;
        sig->second.measurementValue.timer = time;
        producer.produce(serial_number, ObisData::SignedActivePowerL3.measurementType, ObisData::SignedActivePowerL3.wire, sig->second.measurementValue.value);
    }

    // calculate signed total power
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerTotal.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerTotal.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerTotal.toKey())) != end) {
        sig->second.measurementValue.value = pos->second.measurementValue.value - neg->second.measurementValue.value;
        sig->second.measurementValue.timer = time;
        producer.produce(serial_number, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, sig->second.measurementValue.value);
    }

    producer.flush();
}


/**
 * Callback to notify that the last data in the inverter packet has been processed.
 * @param serial The serial number of the originating inverter device.
 * @param time The timestamp associated with the just finished inverter packet.
 */
void CalculatedValueProcessor::endOfSpeedwireData(const uint32_t serial_number, const uint32_t time) {
    SpeedwireDataMap::const_iterator value1, value2, value3, end = speedwire_data_map.end();
    SpeedwireDataMap::iterator result;
    double dc_total = 0.0;
    double ac_total = 0.0;

    // FIXME: THIS NEEDS TO CHECK IF THE VALUES FROM THE MAP ARE TEMPORALLY CLOSE TO TIME

    // calculate total dc power
    if ((value1 = speedwire_data_map.find(SpeedwireData::InverterPowerMPP1.toKey())) != end &&
        (value2 = speedwire_data_map.find(SpeedwireData::InverterPowerMPP2.toKey())) != end) {
        dc_total = value1->second.measurementValue.value + value2->second.measurementValue.value;
        producer.produce(serial_number, SpeedwireData::InverterPowerDCTotal.measurementType, SpeedwireData::InverterPowerDCTotal.wire, dc_total);
    }

    // calculate total ac power
    if ((value1 = speedwire_data_map.find(SpeedwireData::InverterPowerL1.toKey())) != end &&
        (value2 = speedwire_data_map.find(SpeedwireData::InverterPowerL2.toKey())) != end &&
        (value3 = speedwire_data_map.find(SpeedwireData::InverterPowerL3.toKey())) != end) {
        ac_total = value1->second.measurementValue.value + value2->second.measurementValue.value + value3->second.measurementValue.value;
        producer.produce(serial_number, SpeedwireData::InverterPowerACTotal.measurementType, SpeedwireData::InverterPowerACTotal.wire, ac_total);
    }

    // calculate total power loss
    double loss = dc_total - ac_total;
    //if (loss.time != 0)
    producer.produce(serial_number, SpeedwireData::InverterPowerLoss.measurementType, SpeedwireData::InverterPowerLoss.wire, loss);

    // calculate total power efficiency
    double efficiency = (dc_total > 0 ? (ac_total / dc_total) * 100.0 : 0.0);
    //if (loss.time != 0)
    producer.produce(serial_number, SpeedwireData::InverterPowerEfficiency.measurementType, SpeedwireData::InverterPowerEfficiency.wire, efficiency);

    // calculate total power consumption of the house: positive power from grid + inverter power - negative power to grid
    ObisDataMap::const_iterator pos, neg;
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerTotal.toKey())) != obis_data_map.end() &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerTotal.toKey())) != obis_data_map.end()) {
        double household = pos->second.measurementValue.value + ac_total - neg->second.measurementValue.value;
        producer.produce(0xcafebabe, SpeedwireData::HouseholdPowerTotal.measurementType, SpeedwireData::HouseholdPowerTotal.wire, household);

        // calculate monetary income from grid feed and savings from self-consumption
        double feed_in          = neg->second.measurementValue.value * (0.09 / 1000.0);             // assuming  9 cents per kWh
        double self_consumption = (ac_total - neg->second.measurementValue.value) * (0.30 / 1000);  // assuming 30 cents per kWh
        double total            = feed_in + self_consumption;
        producer.produce(0xcafebabe, SpeedwireData::HouseholdIncomeFeedIn.measurementType,          SpeedwireData::HouseholdIncomeFeedIn.wire, feed_in);
        producer.produce(0xcafebabe, SpeedwireData::HouseholdIncomeSelfConsumption.measurementType, SpeedwireData::HouseholdIncomeSelfConsumption.wire, self_consumption);
        producer.produce(0xcafebabe, SpeedwireData::HouseholdIncomeTotal.measurementType,           SpeedwireData::HouseholdIncomeTotal.wire, total);
    }

    producer.flush();
}
