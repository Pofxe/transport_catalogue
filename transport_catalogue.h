#pragma once

#include "domain.h"

#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

namespace transport
{
    typedef  std::unordered_map<std::string_view, Stop*> StopMap;
    typedef  std::unordered_map<std::string_view, Bus*> BusMap;
    typedef  std::unordered_map<std::pair<const Stop*, const  Stop*>, int, StopDistancesHasher> DistanceMap;

    class TransportCatalogue
    {
    public:

        void AddBus(Bus bus_);
        void AddStop(Stop stop_);
        void AddDistance(const std::vector<Distance>& distances_);

        Bus* GetBus(const std::string_view bus_name_);
        Stop* GetStop(const std::string_view stop_name_);

        BusMap GetBusnameToBus() const;
        StopMap GetStopnameToStop() const;

        std::unordered_set<const Bus*> StopGetUniqBuses(Stop* stop_);
        std::unordered_set<const Stop*> GetUniqStops(Bus* bus_);
        double GetLength(Bus* bus_);

        size_t GetDistanceToBus(Bus* bus_);
        size_t GetDistanceStop(const Stop* start_, const Stop* finish_) const;

        std::vector<geo::Coordinates> GetStopCoordinates() const;
        std::vector<std::string_view> GetSortedBusesNames() const;

    private:

        std::deque<Stop> stops;
        StopMap stopname_to_stop;

        std::deque<Bus> buses;
        BusMap busname_to_bus;

        DistanceMap distance_to_stop;
    };

} //end namespace transport