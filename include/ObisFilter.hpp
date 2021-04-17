#ifndef __OBISFILTER_HPP__
#define __OBISFILTER_HPP__

#include <cstdint>
#include <vector>
#include <ObisData.hpp>


class ObisConsumer {
public:
    virtual void consume(ObisData &element) = 0;
};


class ObisFilter {

protected:
    std::vector<ObisConsumer*> consumerTable;
    std::vector<ObisData>      filterTable;

public:
    ObisFilter(void);
    ~ObisFilter(void);

    void addFilter(const ObisData& entry);
    void addFilter(const std::vector<ObisData> &entries);
    void removeFilter(const ObisData &entry);
    const std::vector<ObisData> &getFilter(void) const;

    void addConsumer   (ObisConsumer *obisConsumer);
    void removeConsumer(ObisConsumer *obisConsumer);

    bool consume(const void *const obis, const uint32_t timer);
    ObisData *const filter(const ObisType &element);
    void produce(ObisData &element);
};

#endif
