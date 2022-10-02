#include <CalculatedValueProcessor.hpp>
#include <LocalHost.hpp>
#include <SpeedwireTime.hpp>
using namespace libspeedwire;


// Calculate difference between all positive and negative measurement values and store it in diff values
static void calculateValueDiffs(MeasurementValues& diff, const MeasurementValues& pos, const MeasurementValues& neg) {
    diff.clear();
    for (size_t i = 0; i < pos.getNumberOfMeasurements(); ++i) {
        if (pos.values[i].time == neg.values[i].time) {
            double sig = pos.values[i].value - neg.values[i].value;
            diff.addMeasurement(sig, pos.values[i].time);
        }
    }
}


/**
 * Constructor of the CalculatedValueProcessor instance.
 * @param obis_map       Reference to the data map, where all received obis values reside.
 * @param speedwire_map  Reference to the data map, where all received inverter values reside.
 * @param _producer      Reference to producer to receive the consumed and calculated values.
 */
CalculatedValueProcessor::CalculatedValueProcessor(ObisDataMap& obis_map, SpeedwireDataMap& speedwire_map, Producer& _producer) :
    obis_data_map(obis_map),
    speedwire_data_map(speedwire_map),
    producer(_producer) {
}


/**
 * Destructor.
 */
CalculatedValueProcessor::~CalculatedValueProcessor(void) { }


/**
 * Callback to produce the given obis data to the next stage in the processing pipeline.
 * @param serial_number The serial number of the originating emeter device.
 * @param element A reference to a received ObisData instance, holding output data of the ObisFilter.
 */
void CalculatedValueProcessor::consume(const uint32_t serial_number, ObisData& element) {
    producer.produce(serial_number, element.measurementType, element.wire, element.measurementValues.calculateAverageValue(), element.measurementValues.getMostRecentMeasurement().time);
}


/**
 * Consume a speedwire reply data element
 * @param serial_number The serial number of the originating inverter device.
 * @param element A reference to a received SpeedwireData instance.
 */
void CalculatedValueProcessor::consume(const uint32_t serial_number, SpeedwireData& element) {
    producer.produce(serial_number, element.measurementType, element.wire, element.measurementValues.calculateAverageValue(), element.measurementValues.getMostRecentMeasurement().time);
}


/**
 * Callback to notify that the last obis data in the emeter packet has been processed.
 * @param serial_number The serial number of the originating emeter device.
 * @param timestamp The timestamp associated with the just finished emeter packet.
 */
void CalculatedValueProcessor::endOfObisData(const uint32_t serial_number, const uint32_t timestamp) {
    ObisDataMap::const_iterator pos, neg, end = obis_data_map.end();
    ObisDataMap::iterator sig;

    // calculate signed power L1
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL1.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL1.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL1.toKey())) != end) {
        calculateValueDiffs(sig->second.measurementValues, pos->second.measurementValues, neg->second.measurementValues);
        producer.produce(serial_number, ObisData::SignedActivePowerL1.measurementType, ObisData::SignedActivePowerL1.wire, sig->second.measurementValues.calculateAverageValue(), timestamp);
    }

    // calculate signed power L2
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL2.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL2.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL2.toKey())) != end) {
        calculateValueDiffs(sig->second.measurementValues, pos->second.measurementValues, neg->second.measurementValues);
        producer.produce(serial_number, ObisData::SignedActivePowerL2.measurementType, ObisData::SignedActivePowerL2.wire, sig->second.measurementValues.calculateAverageValue(), timestamp);
    }

    // calculate signed power L3
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL3.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL3.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL3.toKey())) != end) {
        calculateValueDiffs(sig->second.measurementValues, pos->second.measurementValues, neg->second.measurementValues);
        producer.produce(serial_number, ObisData::SignedActivePowerL3.measurementType, ObisData::SignedActivePowerL3.wire, sig->second.measurementValues.calculateAverageValue(), timestamp);
    }

    // calculate signed total power
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerTotal.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerTotal.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerTotal.toKey())) != end) {
        calculateValueDiffs(sig->second.measurementValues, pos->second.measurementValues, neg->second.measurementValues);
        producer.produce(serial_number, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, sig->second.measurementValues.calculateAverageValue(), timestamp);
    }

    producer.flush();
}


/**
 * Callback to notify that the last data in the inverter packet has been processed.
 * @param serial_number The serial number of the originating inverter device.
 * @param timestamp The unix epoch time associated with the just finished inverter packet.
 */
void CalculatedValueProcessor::endOfSpeedwireData(const uint32_t serial_number, const uint32_t timestamp) {
    SpeedwireDataMap::const_iterator value1, value2, value3, end = speedwire_data_map.end();
    uint32_t value1_time, value2_time, value3_time;
    double dc_total = 0.0;
    double ac_total = 0.0;
    static const uint32_t max_age = 120;       // maximum age of data in seconds
    uint32_t dc_age = max_age;
    uint32_t ac_age = max_age;
    uint32_t dc_time = 0;
    uint32_t ac_time = 0;

    // get current time to check the age of data
    uint64_t current_time  = LocalHost::getUnixEpochTimeInMs();
    uint32_t inverter_time = (uint32_t)(current_time / 1000);   // inverter timestamps are in seconds
    uint32_t emeter_time   = (uint32_t)current_time;            // emeter timestamps are in milliseconds

    // calculate total dc power
    if ((value1 = speedwire_data_map.find(SpeedwireData::InverterPowerMPP1.toKey())) != end &&
        (value2 = speedwire_data_map.find(SpeedwireData::InverterPowerMPP2.toKey())) != end &&
        (value1_time = value1->second.measurementValues.getMostRecentMeasurement().time, 
         value2_time = value2->second.measurementValues.getMostRecentMeasurement().time,
         LocalHost::calculateAbsTimeDifference(value1_time, value2_time) <= 1)) {
        dc_age = (uint32_t)LocalHost::calculateAbsTimeDifference(inverter_time, value1_time);
        dc_time = value1_time;
        //if (dc_age < max_age) {
            dc_total = value1->second.measurementValues.calculateAverageValue() + value2->second.measurementValues.calculateAverageValue();
            producer.produce(serial_number, SpeedwireData::InverterPowerDCTotal.measurementType, SpeedwireData::InverterPowerDCTotal.wire, dc_total, value1_time);
        //}
    }

    // calculate total ac power
    if ((value1 = speedwire_data_map.find(SpeedwireData::InverterPowerL1.toKey())) != end &&
        (value2 = speedwire_data_map.find(SpeedwireData::InverterPowerL2.toKey())) != end &&
        (value3 = speedwire_data_map.find(SpeedwireData::InverterPowerL3.toKey())) != end &&
        (value1_time = value1->second.measurementValues.getMostRecentMeasurement().time,
         value2_time = value2->second.measurementValues.getMostRecentMeasurement().time,
         value3_time = value3->second.measurementValues.getMostRecentMeasurement().time,
         LocalHost::calculateAbsTimeDifference(value1_time, value2_time) <= 1 &&
         LocalHost::calculateAbsTimeDifference(value1_time, value3_time) <= 1)) {
        ac_age = (uint32_t)LocalHost::calculateAbsTimeDifference(inverter_time, value1_time);
        ac_time = value1_time;
        //if (ac_age < max_age) {
            ac_total = value1->second.measurementValues.calculateAverageValue() + value2->second.measurementValues.calculateAverageValue() + value3->second.measurementValues.calculateAverageValue();
            producer.produce(serial_number, SpeedwireData::InverterPowerACTotal.measurementType, SpeedwireData::InverterPowerACTotal.wire, ac_total, value1_time);
        //}

        if (LocalHost::calculateAbsTimeDifference(dc_age, ac_age) <= 2) {
            // calculate total power loss
            double loss = dc_total - ac_total;
            producer.produce(serial_number, SpeedwireData::InverterPowerLoss.measurementType, SpeedwireData::InverterPowerLoss.wire, loss, value1_time);

            // calculate total power efficiency
            double efficiency = (dc_total > 0 ? (ac_total / dc_total) * 100.0 : 0.0);
            producer.produce(serial_number, SpeedwireData::InverterPowerEfficiency.measurementType, SpeedwireData::InverterPowerEfficiency.wire, efficiency, value1_time);
        }
    }

    ObisDataMap::const_iterator pos, neg;
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerTotal.toKey())) != obis_data_map.end() &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerTotal.toKey())) != obis_data_map.end()) {
        uint32_t feed_in_time = neg->second.measurementValues.getMostRecentMeasurement().time;
        uint32_t grid_age = emeter_time - feed_in_time;
        if (grid_age < max_age * 1000) {
            double neg_average_value = neg->second.measurementValues.calculateAverageValue();

            // calculate total power consumption of the house: positive power from grid + inverter power - negative power to grid
            double household;
            if (ac_total == 0.0) {
                household = pos->second.measurementValues.calculateAverageValue() - neg_average_value;
            } else {
                uint32_t ac_time_emeter = SpeedwireTime::convertInverterToEmeterTime(ac_time, current_time);
                household = pos->second.measurementValues.findClosestMeasurement(ac_time_emeter).value + ac_total - neg->second.measurementValues.findClosestMeasurement(ac_time_emeter).value;
                if (household < 0.0) household = 0.0;  // this can happen if there is a steep change in solar production or energy consumption and measurements are taken at different points in time
            }
            producer.produce(0xcafebabe, SpeedwireData::HouseholdPowerTotal.measurementType, SpeedwireData::HouseholdPowerTotal.wire, household, feed_in_time);

            // calculate monetary income from grid feed and savings from self-consumption
            double feed_in          = neg_average_value * (0.09 / 1000.0);             // assuming  9 cents per kWh
            double self_consumption = (ac_total - neg_average_value) * (0.30 / 1000);  // assuming 30 cents per kWh
            double total            = feed_in + self_consumption;
            producer.produce(0xcafebabe, SpeedwireData::HouseholdIncomeFeedIn.measurementType,          SpeedwireData::HouseholdIncomeFeedIn.wire,          feed_in, feed_in_time);
            producer.produce(0xcafebabe, SpeedwireData::HouseholdIncomeSelfConsumption.measurementType, SpeedwireData::HouseholdIncomeSelfConsumption.wire, self_consumption, feed_in_time);
            producer.produce(0xcafebabe, SpeedwireData::HouseholdIncomeTotal.measurementType,           SpeedwireData::HouseholdIncomeTotal.wire,           total, feed_in_time);
        }
    }

    producer.flush();
}
