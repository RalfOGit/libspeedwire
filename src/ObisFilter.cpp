#include <ObisFilter.hpp>
#include <SpeedwireEmeterProtocol.hpp>


ObisFilter::ObisFilter(void) {
    // nothing to do
}

ObisFilter::~ObisFilter(void) {
    filterTable.clear();
    consumerTable.clear();
}

void ObisFilter::addFilter(const ObisData &entry) {
    filterTable.push_back(entry);
}

void ObisFilter::addFilter(const std::vector<ObisData> &entries) {
    for (std::vector<ObisData>::const_iterator it = entries.begin(); it != entries.end(); it++) {
        addFilter(*it);
    }
}

void ObisFilter::removeFilter(const ObisData &entry) {
    for (std::vector<ObisData>::iterator it = filterTable.begin(); it != filterTable.end(); it++) {
        if (it->equals(entry)) {
            it = filterTable.erase(it);
        }
    }
}

const std::vector<ObisData> &ObisFilter::getFilter(void) const {
    return filterTable;
}

void ObisFilter::addConsumer(ObisConsumer *obisConsumer) {
    consumerTable.push_back(obisConsumer);
}

void ObisFilter::removeConsumer(ObisConsumer *obisConsumer) {
   for (std::vector<ObisConsumer*>::iterator it = consumerTable.begin(); it != consumerTable.end(); it++) {
        if (*it == obisConsumer) {
            it = consumerTable.erase(it);
        }
    }
}

bool ObisFilter::consume(const void *const obis, const uint32_t timer) const {
    ObisType element(SpeedwireEmeterProtocol::getObisChannel(obis),
                     SpeedwireEmeterProtocol::getObisIndex(obis),
                     SpeedwireEmeterProtocol::getObisType(obis),
                     SpeedwireEmeterProtocol::getObisTariff(obis));

    const ObisData *const filteredElement = filter(element);
    if (filteredElement != NULL && filteredElement->measurementValue != NULL) {
        MeasurementValue *mvalue = filteredElement->measurementValue;
        switch (filteredElement->type) {
        case 0:
            mvalue->setValue(SpeedwireEmeterProtocol::toValueString(obis, false));
            break;
        case 4:
            mvalue->setValue((uint32_t)SpeedwireEmeterProtocol::getObisValue4(obis), filteredElement->measurementType.divisor);
            break;
        case 7:
            mvalue->setValue((int32_t)SpeedwireEmeterProtocol::getObisValue4(obis), filteredElement->measurementType.divisor);
            break;
        case 8:
            mvalue->setValue(SpeedwireEmeterProtocol::getObisValue8(obis), filteredElement->measurementType.divisor);
            break;
        default:
            perror("obis identifier not implemented");
        }
        mvalue->setTimer(timer);
        produce(*filteredElement);

        return true;
    }
    return false;
}

const ObisData *const ObisFilter::filter(const ObisType &element) const {
    for (std::vector<ObisData>::const_iterator it = filterTable.begin(); it != filterTable.end(); it++) {
        if (it->equals(element)) {
            return &(*it);
        }
    }
    return NULL;
}

void ObisFilter::produce(const ObisData &element) const {
    for (std::vector<ObisConsumer*>::const_iterator it = consumerTable.begin(); it != consumerTable.end(); it++) {
        (*it)->consume(element);
    }
}
