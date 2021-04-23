#ifndef __OBISFILTER_HPP__
#define __OBISFILTER_HPP__

#include <cstdint>
#include <vector>
#include <Consumer.hpp>
#include <ObisData.hpp>


/**
 *  Class ObisFilter implements the filtering of obis elements.
 *
 *  The type of ObisData that is to be filtered out, can be configured by adding elements to the filter.
 *  Consumers of filtered obis data can register themselves to the filter.
 *  The general idea is that the ObisData instances held by the filter will hold the most recent obis data
 *  values. Also aggregation of consecutively received obis data is done inside the ObisData instances held
 *  by the filter. Registered onsumers will recieve a reference to the ObisData instance held by the filter.
 */
class ObisFilter {

protected:
    std::vector<ObisConsumer*> consumerTable;   //!< Table of registered ObisConsumers
    std::vector<ObisData>      filterTable;     //!< Table of registered ObisData instance

public:
    ObisFilter(void);
    ~ObisFilter(void);

    void addFilter(const ObisData& entry);
    void addFilter(const std::vector<ObisData> &entries);
    void removeFilter(const ObisData &entry);
    const std::vector<ObisData> &getFilter(void) const;

    void addConsumer   (ObisConsumer *obisConsumer);
    void removeConsumer(ObisConsumer *obisConsumer);

    bool consume(const uint32_t serial, const void *const obis, const uint32_t timer);
    ObisData *const filter(const uint32_t serial, const ObisType &element);
    void produce(const uint32_t serial, ObisData &element);
};

#endif
