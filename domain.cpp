#include "domain.h"

bool PtrComp::operator()(const Bus* lhs, const Bus* rhs) const {
    return lhs->bus_name < rhs->bus_name;
}

size_t StopHasher::operator() (std::pair<Stop*, Stop*> val) const {
    return std::hash<const void*>()(val.first) ^ std::hash<const void*>()(val.second);
}
