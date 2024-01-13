#include <CalculatedValueProcessor.hpp>
#include <LocalHost.hpp>
#include <SpeedwireTime.hpp>
#include <LineSegmentEstimator.hpp>
using namespace libspeedwire;


// Calculate difference between all positive and negative measurement values and store it in diff values
static void calculateValueDiffs(Measurement& diff, const Measurement& pos, const Measurement& neg) {
    const MeasurementValues& pos_values = pos.measurementValues;
    const MeasurementValues& neg_values = neg.measurementValues;
    MeasurementValues& diff_values = diff.measurementValues;
    diff_values.clear();
    for (size_t i = 0; i < pos_values.getNumberOfElements(); ++i) {
        if (pos_values[i].time == neg_values[i].time) {
            double signed_value = pos_values[i].value - neg_values[i].value;
            diff_values.addMeasurement(signed_value, pos_values[i].time);
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
 * @param device The originating inverter device.
 * @param element A reference to a received ObisData instance, holding output data of the ObisFilter.
 */
void CalculatedValueProcessor::consume(const SpeedwireDevice& device, ObisData& element) {
    producer.produce(device, element.measurementType, element.wire, element.measurementValues.estimateMean(), element.measurementValues.getNewestElement().time);
}


/**
 * Consume a speedwire reply data element
 * @param device The originating inverter device.
 * @param element A reference to a received SpeedwireData instance.
 */
void CalculatedValueProcessor::consume(const SpeedwireDevice& device, SpeedwireData& element) {
    producer.produce(device, element.measurementType, element.wire, element.measurementValues.estimateMean(), element.measurementValues.getNewestElement().time);
}


/**
 * Callback to notify that the last obis data in the emeter packet has been processed.
 * @param device The originating inverter device.
 * @param timestamp The timestamp associated with the just finished emeter packet.
 */
void CalculatedValueProcessor::endOfObisData(const SpeedwireDevice& device, const uint32_t timestamp) {
    ObisDataMap::const_iterator pos, neg, end = obis_data_map.end();
    ObisDataMap::iterator sig;

    // calculate signed power L1
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL1.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL1.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL1.toKey())) != end) {
        calculateValueDiffs(sig->second, pos->second, neg->second);
        producer.produce(device, ObisData::SignedActivePowerL1.measurementType, ObisData::SignedActivePowerL1.wire, sig->second.measurementValues.estimateMean(), timestamp);
    }

    // calculate signed power L2
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL2.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL2.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL2.toKey())) != end) {
        calculateValueDiffs(sig->second, pos->second, neg->second);
        producer.produce(device, ObisData::SignedActivePowerL2.measurementType, ObisData::SignedActivePowerL2.wire, sig->second.measurementValues.estimateMean(), timestamp);
    }

    // calculate signed power L3
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerL3.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerL3.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerL3.toKey())) != end) {
        calculateValueDiffs(sig->second, pos->second, neg->second);
        producer.produce(device, ObisData::SignedActivePowerL3.measurementType, ObisData::SignedActivePowerL3.wire, sig->second.measurementValues.estimateMean(), timestamp);
    }

    // calculate signed total power
    if ((pos = obis_data_map.find(ObisData::PositiveActivePowerTotal.toKey())) != end &&
        (neg = obis_data_map.find(ObisData::NegativeActivePowerTotal.toKey())) != end &&
        (sig = obis_data_map.find(ObisData::SignedActivePowerTotal.toKey())) != end) {
        calculateValueDiffs(sig->second, pos->second, neg->second);
        producer.produce(device, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, sig->second.measurementValues.estimateMean(), timestamp);

#if 1
        // experimental setup to feed time-accurate power measurements
        static uint32_t last_time = 0;
        SpeedwireDevice experimental_device;
        experimental_device.deviceAddress.serialNumber = 1234567890;
        const MeasurementValues& mvalues = sig->second.measurementValues;
        std::vector<MeasurementValueInterval> intervals;
        LineSegmentEstimator::findPiecewiseConstantIntervals(mvalues, intervals);
        for (auto& iv : intervals) {
            if (SpeedwireTime::calculateTimeDifference(mvalues[iv.end_index].time, last_time) <= 0) {
#ifdef _DEBUG
                printf("interval %d %d - %lu %lu : %lf  => skipped\n", (int)iv.start_index, (int)iv.end_index, mvalues[iv.start_index].time, mvalues[iv.end_index].time, iv.mean_value);
#endif
                continue;
            }
            while (SpeedwireTime::calculateTimeDifference(mvalues[iv.start_index].time, last_time) <= 0 && iv.start_index < (mvalues.getNumberOfElements() - 1)) {
#ifdef _DEBUG
                printf("interval %d %d - %lu %lu : %lf  => incremented start index\n", (int)iv.start_index, (int)iv.end_index, mvalues[iv.start_index].time, mvalues[iv.end_index].time, iv.mean_value);
#endif
                iv.start_index++;
            }
#ifdef _DEBUG
            printf("interval %d %d - %lu %lu : %lf\n", (int)iv.start_index, (int)iv.end_index, mvalues[iv.start_index].time, mvalues[iv.end_index].time, iv.mean_value);
#endif
            producer.produce(experimental_device, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, iv.mean_value, mvalues[iv.start_index].time);
            producer.produce(experimental_device, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, iv.mean_value, mvalues[iv.end_index].time);
        }
        last_time = mvalues[intervals[intervals.size() - 1].end_index].time;
#else
        static MeasurementValues experimentalValues(1024);

        // experimental setup to feed time-accurate power measurements
        for (size_t i = 0; i < sig->second.measurementValues.getNumberOfElements(); ++i) {
            const TimestampDoublePair& pair = sig->second.measurementValues.at(i);
            experimentalValues.addMeasurement(pair.value, pair.time);
        }
        static uint32_t last_time = 0;
        SpeedwireDevice experimental_device;
        experimental_device.serialNumber = 1234567890;
        std::vector<MeasurementValueInterval> intervals;
        if (LineSegmentEstimator::findPiecewiseConstantIntervals(experimentalValues, intervals) > 0) {
            intervals.erase(intervals.end() - 1);
            for (auto& iv : intervals) {
                if (SpeedwireTime::calculateTimeDifference(experimentalValues[iv.end_index].time, last_time) <= 0) {
#ifdef _DEBUG
                    printf("interval %d %d - %lu %lu : %lf  => skipped\n", (int)iv.start_index, (int)iv.end_index, experimentalValues[iv.start_index].time, experimentalValues[iv.end_index].time, iv.mean_value);
#endif
                    continue;
                }
                while (SpeedwireTime::calculateTimeDifference(experimentalValues[iv.start_index].time, last_time) <= 0 && iv.start_index < (experimentalValues.getNumberOfElements() - 1)) {
#ifdef _DEBUG
                    printf("interval %d %d - %lu %lu : %lf  => incremented start index\n", (int)iv.start_index, (int)iv.end_index, experimentalValues[iv.start_index].time, experimentalValues[iv.end_index].time, iv.mean_value);
#endif
                    iv.start_index++;
                }
#ifdef _DEBUG
                printf("interval %d %d - %lu %lu : %lf\n", (int)iv.start_index, (int)iv.end_index, experimentalValues[iv.start_index].time, experimentalValues[iv.end_index].time, iv.mean_value);
#endif
                producer.produce(experimental_device, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, iv.mean_value, experimentalValues[iv.start_index].time);
                producer.produce(experimental_device, ObisData::SignedActivePowerTotal.measurementType, ObisData::SignedActivePowerTotal.wire, iv.mean_value, experimentalValues[iv.end_index].time);
            }
        }

        //std::vector<MeasurementValueInterval> intervals2;
        //LineSegmentEstimator::findPiecewiseLinearIntervals(experimentalValues, intervals2);

        // clear all measurements that have been sent to the producer
        if (intervals.size() > 0) {
            size_t end_index = intervals[intervals.size() - 1].end_index;
            last_time = experimentalValues[end_index].time;
            experimentalValues.removeElements(0, end_index + 1);
        }
#endif
    }

    producer.flush();
}


/**
 * Callback to notify that the last data in the inverter packet has been processed.
 * @param serial_number The serial number of the originating inverter device.
 * @param timestamp The unix epoch time associated with the just finished inverter packet.
 */
void CalculatedValueProcessor::endOfSpeedwireData(const SpeedwireDevice& device, const uint32_t timestamp) {
    SpeedwireDataMap::const_iterator value1, value2, value3, end = speedwire_data_map.end();
    uint32_t value1_time, value2_time, value3_time;
    double dc_total = 0.0;
    double ac_total = 0.0;
    static const uint32_t max_age = 120;       // maximum age of data in seconds
    uint32_t dc_age = max_age;
    uint32_t ac_age = max_age;
    uint32_t dc_time = 0;
    uint32_t ac_time = 0;

    if (device.deviceClass == "Battery-Inverter") {
        // calculate total battery inverter ac power
        if ((value1 = speedwire_data_map.find(SpeedwireData::BatteryPowerL1.toKey())) != end &&
            (value2 = speedwire_data_map.find(SpeedwireData::BatteryPowerL2.toKey())) != end &&
            (value3 = speedwire_data_map.find(SpeedwireData::BatteryPowerL3.toKey())) != end &&
            (value1_time = value1->second.measurementValues.getNewestElement().time,
                value2_time = value2->second.measurementValues.getNewestElement().time,
                value3_time = value3->second.measurementValues.getNewestElement().time,
                SpeedwireTime::calculateAbsTimeDifference(value1_time, value2_time) <= 1 &&
                SpeedwireTime::calculateAbsTimeDifference(value1_time, value3_time) <= 1)) {
            ac_total = value1->second.measurementValues.estimateMean() + value2->second.measurementValues.estimateMean() + value3->second.measurementValues.estimateMean();
            producer.produce(device, SpeedwireData::BatteryPowerACTotal.measurementType, SpeedwireData::BatteryPowerACTotal.wire, ac_total, value1_time);
        }
    }
    else {
        // get current time to check the age of data
        uint64_t current_time = LocalHost::getUnixEpochTimeInMs();
        uint32_t inverter_time = (uint32_t)(current_time / 1000);   // inverter timestamps are in seconds
        uint32_t emeter_time = (uint32_t)current_time;            // emeter timestamps are in milliseconds

        // calculate total dc power
        if ((value1 = speedwire_data_map.find(SpeedwireData::InverterPowerMPP1.toKey())) != end &&
            (value2 = speedwire_data_map.find(SpeedwireData::InverterPowerMPP2.toKey())) != end &&
            (value1_time = value1->second.measurementValues.getNewestElement().time,
                value2_time = value2->second.measurementValues.getNewestElement().time,
                SpeedwireTime::calculateAbsTimeDifference(value1_time, value2_time) <= 1)) {
            dc_age = (uint32_t)SpeedwireTime::calculateAbsTimeDifference(inverter_time, value1_time);
            dc_time = value1_time;
            //if (dc_age < max_age) {
            dc_total = value1->second.measurementValues.estimateMean() + value2->second.measurementValues.estimateMean();
            producer.produce(device, SpeedwireData::InverterPowerDCTotal.measurementType, SpeedwireData::InverterPowerDCTotal.wire, dc_total, value1_time);
            //}
        }

        // calculate total pv inverter ac power
        if ((value1 = speedwire_data_map.find(SpeedwireData::InverterPowerL1.toKey())) != end &&
            (value2 = speedwire_data_map.find(SpeedwireData::InverterPowerL2.toKey())) != end &&
            (value3 = speedwire_data_map.find(SpeedwireData::InverterPowerL3.toKey())) != end &&
            (value1_time = value1->second.measurementValues.getNewestElement().time,
                value2_time = value2->second.measurementValues.getNewestElement().time,
                value3_time = value3->second.measurementValues.getNewestElement().time,
                SpeedwireTime::calculateAbsTimeDifference(value1_time, value2_time) <= 1 &&
                SpeedwireTime::calculateAbsTimeDifference(value1_time, value3_time) <= 1)) {
            ac_age = (uint32_t)SpeedwireTime::calculateAbsTimeDifference(inverter_time, value1_time);
            ac_time = value1_time;
            //if (ac_age < max_age) {
            ac_total = value1->second.measurementValues.estimateMean() + value2->second.measurementValues.estimateMean() + value3->second.measurementValues.estimateMean();
            producer.produce(device, SpeedwireData::InverterPowerACTotal.measurementType, SpeedwireData::InverterPowerACTotal.wire, ac_total, value1_time);
            //}

            if (SpeedwireTime::calculateAbsTimeDifference(dc_age, ac_age) <= 2) {
                // calculate total power loss
                double loss = dc_total - ac_total;
                producer.produce(device, SpeedwireData::InverterPowerLoss.measurementType, SpeedwireData::InverterPowerLoss.wire, loss, value1_time);

                // calculate total power efficiency
                double efficiency = (dc_total > 0 ? (ac_total / dc_total) * 100.0 : 0.0);
                producer.produce(device, SpeedwireData::InverterPowerEfficiency.measurementType, SpeedwireData::InverterPowerEfficiency.wire, efficiency, value1_time);
            }
        }

        ObisDataMap::const_iterator pos, neg;
        if ((pos = obis_data_map.find(ObisData::PositiveActivePowerTotal.toKey())) != obis_data_map.end() &&
            (neg = obis_data_map.find(ObisData::NegativeActivePowerTotal.toKey())) != obis_data_map.end()) {
            uint32_t feed_in_time = neg->second.measurementValues.getNewestElement().time;
            uint32_t grid_age = SpeedwireTime::calculateAbsTimeDifference(emeter_time, feed_in_time);
            if (grid_age < max_age * 1000) {
                double neg_average_value = neg->second.measurementValues.estimateMean();

                // calculate total power consumption of the house: positive power from grid + inverter power - negative power to grid
                double household;
                if (ac_total == 0.0) {
                    household = pos->second.measurementValues.estimateMean() - neg_average_value;
                }
                else {
                    uint32_t ac_time_emeter = SpeedwireTime::convertInverterToEmeterTime(ac_time, current_time);
                    //household = pos->second.measurementValues.findClosestMeasurement(ac_time_emeter).value + ac_total - neg->second.measurementValues.findClosestMeasurement(ac_time_emeter).value;
                    household = pos->second.measurementValues.interpolateClosestValues(ac_time_emeter) + ac_total - neg->second.measurementValues.interpolateClosestValues(ac_time_emeter);
                    if (household < 0.0) household = 0.0;  // this can happen if there is a steep change in solar production or energy consumption and measurements are taken at different points in time
                }
                // consider battery inverter power: household power + battery inverter power
                if ((value1 = speedwire_data_map.find(SpeedwireData::BatteryPowerACTotal.toKey())) != end &&
                    (value1_time = value1->second.measurementValues.getNewestElement().time)) {
                    uint32_t bat_ac_age = (uint32_t)SpeedwireTime::calculateAbsTimeDifference(inverter_time, value1_time);
                    if (SpeedwireTime::calculateAbsTimeDifference(bat_ac_age, ac_age) <= 10) {
                        household += value1->second.measurementValues.interpolateClosestValues(ac_time);
                        if (household < 0.0) household = 0.0;  // this can happen if there is a steep change in solar production or energy consumption and measurements are taken at different points in time
                    }
                }

                SpeedwireDevice household_device;
                household_device.deviceAddress.serialNumber = 0xcafebabe;

                producer.produce(household_device, SpeedwireData::HouseholdPowerTotal.measurementType, SpeedwireData::HouseholdPowerTotal.wire, household, feed_in_time);

                // calculate monetary income from grid feed and savings from self-consumption
                double feed_in = neg_average_value * (0.09 / 1000.0);             // assuming  9 cents per kWh
                double self_consumption = (ac_total - neg_average_value) * (0.30 / 1000);  // assuming 30 cents per kWh
                double total = feed_in + self_consumption;
                producer.produce(household_device, SpeedwireData::HouseholdIncomeFeedIn.measurementType, SpeedwireData::HouseholdIncomeFeedIn.wire, feed_in, feed_in_time);
                producer.produce(household_device, SpeedwireData::HouseholdIncomeSelfConsumption.measurementType, SpeedwireData::HouseholdIncomeSelfConsumption.wire, self_consumption, feed_in_time);
                producer.produce(household_device, SpeedwireData::HouseholdIncomeTotal.measurementType, SpeedwireData::HouseholdIncomeTotal.wire, total, feed_in_time);
            }
        }
    }
    producer.flush();
}
