#include "transport_catalogue.h"

#include <execution>

namespace transport
{
    void TransportCatalogue::AddStop(Stop stop_)
    {
        stops.push_back(std::move(stop_));
        Stop* stop_buf = &stops.back();

        stopname_to_stop.insert(transport::StopMap::value_type(stop_buf->name, stop_buf));
    }

    void TransportCatalogue::AddBus(Bus bus_)
    {
        Bus* bus_buf;

        buses.push_back(std::move(bus_));
        bus_buf = &buses.back();

        busname_to_bus.insert(BusMap::value_type(bus_buf->name, bus_buf));

        for (Stop* stop : bus_buf->stops)
        {
            stop->buses.push_back(bus_buf);
        }

        bus_buf->route_length = GetDistanceToBus(bus_buf);
    }

    void TransportCatalogue::AddDistance(const std::vector<Distance>& distances_)
    {
        for (const Distance& distance : distances_)
        {
            auto dist_pair = std::make_pair(distance.start, distance.end);
            distance_to_stop.insert(DistanceMap::value_type(dist_pair, distance.distance));
        }
    }

    Bus* TransportCatalogue::GetBus(const std::string_view bus_name_)
    {
        if (busname_to_bus.empty())
        {
            return nullptr;
        }
        try
        {
            return busname_to_bus.at(bus_name_);
        }
        catch (const std::out_of_range& e)
        {
            return nullptr;
        }
    }

    Stop* TransportCatalogue::GetStop(const std::string_view stop_name_)
    {
        if (stopname_to_stop.empty())
        {
            return nullptr;
        }
        try
        {
            return stopname_to_stop.at(stop_name_);
        }
        catch (const std::out_of_range& e)
        {
            return nullptr;
        }
    }

    BusMap TransportCatalogue::GetBusnameToBus() const
    {
        return busname_to_bus;
    }

    StopMap TransportCatalogue::GetStopnameToStop() const
    {
        return stopname_to_stop;
    }

    std::unordered_set<const Stop*> TransportCatalogue::GetUniqStops(Bus* bus_)
    {
        std::unordered_set<const Stop*> unique_stops;

        unique_stops.insert(bus_->stops.begin(), bus_->stops.end());

        return unique_stops;
    }

    double TransportCatalogue::GetLength(Bus* bus_)
    {
        return transform_reduce(next(bus_->stops.begin()), bus_->stops.end(), bus_->stops.begin(), 0.0, std::plus<>{}, [](const Stop* lhs, const Stop* rhs)
            {
                return geo::ComputeDistance({ (*lhs).latitude, (*lhs).longitude }, { (*rhs).latitude, (*rhs).longitude });
            });
    }

    std::unordered_set<const Bus*> TransportCatalogue::StopGetUniqBuses(Stop* stop_)
    {
        std::unordered_set<const Bus*> unique_stops;

        unique_stops.insert(stop_->buses.begin(), stop_->buses.end());

        return unique_stops;
    }

    size_t TransportCatalogue::GetDistanceStop(const Stop* start_, const Stop* finish_) const
    {
        if (distance_to_stop.count({ start_, finish_ }))
        {
            return distance_to_stop.at({ start_, finish_ });
        }
        else if (distance_to_stop.count({ finish_, start_ }))
        {
            return distance_to_stop.at({ finish_, start_ });
        }
        else
        {
            return 0;
        }
    }

    size_t TransportCatalogue::GetDistanceToBus(Bus* bus_)
    {
        size_t distance = 0;
        size_t stops_size = bus_->stops.size() - 1;

        for (size_t i = 0; i < stops_size; i++)
        {
            distance += GetDistanceStop(bus_->stops[i], bus_->stops[i + 1]);
        }
        return distance;
    }

    std::vector<geo::Coordinates> TransportCatalogue::GetStopCoordinates() const
    {
        std::vector<geo::Coordinates> stops_coordinates;
        transport::BusMap buses = GetBusnameToBus();

        for (const auto& [_, bus] : buses)
        {
            for (const auto& stop : bus->stops)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop->latitude;
                coordinates.longitude = stop->longitude;

                stops_coordinates.push_back(coordinates);
            }
        }
        return stops_coordinates;
    }

    std::vector<std::string_view> TransportCatalogue::GetSortedBusesNames() const
    {
        std::vector<std::string_view> buses_names;

        transport::BusMap buses = GetBusnameToBus();
        if (buses.size() > 0)
        {
            for (const auto& [busname, _] : buses)
            {
                buses_names.push_back(busname);
            }

            std::sort(buses_names.begin(), buses_names.end());

            return buses_names;
        }
        else
        {
            return {};
        }
    }

}//end namespace transport