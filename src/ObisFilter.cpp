#include <ObisFilter.hpp>
#include <SpeedwireEmeterProtocol.hpp>


ObisFilter::ObisFilter(void) {
    // nothing to do
}

ObisFilter::~ObisFilter(void) {
    filterMap.clear();
    consumerTable.clear();
}

void ObisFilter::addFilter(const ObisData &entry) {
    filterMap[entry.toKey()] = entry;
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
    filterMap.erase(entry.toKey());
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

bool ObisFilter::consume(const uint32_t serial, const void *const obis, const uint32_t time) {
    ObisType element(SpeedwireEmeterProtocol::getObisChannel(obis),
                     SpeedwireEmeterProtocol::getObisIndex(obis),
                     SpeedwireEmeterProtocol::getObisType(obis),
                     SpeedwireEmeterProtocol::getObisTariff(obis));

    ObisData *const filteredElement = filter(serial, element);
    if (filteredElement != NULL) {
        MeasurementValue& mvalue = filteredElement->measurementValue;
        switch (filteredElement->type) {
        case 0:
            mvalue.setValue(SpeedwireEmeterProtocol::toValueString(obis, false));
            break;
        case 4:
            mvalue.setValue((uint32_t)SpeedwireEmeterProtocol::getObisValue4(obis), filteredElement->measurementType.divisor);
            break;
        case 7:
            mvalue.setValue((int32_t)SpeedwireEmeterProtocol::getObisValue4(obis), filteredElement->measurementType.divisor);
            break;
        case 8:
            mvalue.setValue(SpeedwireEmeterProtocol::getObisValue8(obis), filteredElement->measurementType.divisor);
            break;
        default:
            perror("obis identifier not implemented");
        }
        mvalue.setTimer(time);
        produce(serial, *filteredElement);

        return true;
    }
    return false;
}

ObisData *const ObisFilter::filter(const uint32_t serial, const ObisType &element) {
    auto& it = filterMap.find(element.toKey());
    if (it != filterMap.end()) {
        return &(it->second);
    }
    return NULL;
}

void ObisFilter::produce(const uint32_t serial, ObisData &element) {
    for (std::vector<ObisConsumer*>::iterator it = consumerTable.begin(); it != consumerTable.end(); it++) {
        (*it)->consume(serial, element);
    }
}

void ObisFilter::endOfObisData(const uint32_t serial, const uint32_t time) {
    for (std::vector<ObisConsumer*>::iterator it = consumerTable.begin(); it != consumerTable.end(); it++) {
        (*it)->endOfObisData(serial, time);
    }
}
