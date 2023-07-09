#include <ObisFilter.hpp>
#include <SpeedwireEmeterProtocol.hpp>
using namespace libspeedwire;


ObisFilter::ObisFilter(void) {
    // nothing to do
}

ObisFilter::~ObisFilter(void) {
    filterMap.clear();
    consumerTable.clear();
}

void ObisFilter::addFilter(const ObisData &entry) {
    ObisData& filter_entry = filterMap[entry.toKey()];
    filter_entry = entry;
    filter_entry.measurementValues.setMaximumNumberOfElements(entry.measurementValues.getMaximumNumberOfElements());
}

void ObisFilter::addFilter(const std::vector<ObisData> &entries) {
    for (std::vector<ObisData>::const_iterator it = entries.begin(); it != entries.end(); it++) {
        addFilter(*it);
    }
}

void ObisFilter::addFilter(const ObisDataMap& entries) {
    for (const auto& entry : entries) {
        addFilter(entry.second);
    }
}

void ObisFilter::removeFilter(const ObisData &entry) {
    filterMap.remove(entry);
}

ObisDataMap& ObisFilter::getFilter(void) {
    return filterMap;
}

/**
 *  Add an obis consumer to receive the result of the ObisFilter.
 */
void ObisFilter::addConsumer(ObisConsumer& obisConsumer) {
    consumerTable.push_back(&obisConsumer);
}

bool ObisFilter::consume(const SpeedwireDevice& device, const void *const obis, const uint32_t time) {
    ObisType element(SpeedwireEmeterProtocol::getObisChannel(obis),
                     SpeedwireEmeterProtocol::getObisIndex(obis),
                     SpeedwireEmeterProtocol::getObisType(obis),
                     SpeedwireEmeterProtocol::getObisTariff(obis));

    ObisData *const filteredElement = filter(device, element);
    if (filteredElement != NULL) {
        switch (filteredElement->type) {
        case 0:
            filteredElement->measurementValues.value_string = SpeedwireEmeterProtocol::toValueString(obis, false);
            break;
        case 4:
            filteredElement->addMeasurement((uint32_t)SpeedwireEmeterProtocol::getObisValue4(obis), time);
            break;
        case 7:
            filteredElement->addMeasurement((int32_t)SpeedwireEmeterProtocol::getObisValue4(obis), time);
            break;
        case 8:
            filteredElement->addMeasurement(SpeedwireEmeterProtocol::getObisValue8(obis), time);
            break;
        default:
            perror("obis identifier not implemented");
        }
        produce(device, *filteredElement);

        return true;
    }
    return false;
}

ObisData *const ObisFilter::filter(const SpeedwireDevice& device, const ObisType &element) {
    const auto& it = filterMap.find(element.toKey());
    if (it != filterMap.end()) {
        return &(it->second);
    }
    return NULL;
}

void ObisFilter::produce(const SpeedwireDevice& device, ObisData &element) {
    for (std::vector<ObisConsumer*>::iterator it = consumerTable.begin(); it != consumerTable.end(); it++) {
        (*it)->consume(device, element);
    }
}

void ObisFilter::endOfObisData(const SpeedwireDevice& device, const uint32_t time) {
    for (std::vector<ObisConsumer*>::iterator it = consumerTable.begin(); it != consumerTable.end(); it++) {
        (*it)->endOfObisData(device, time);
    }
}
