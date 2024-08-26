#include "transport_router.h"

namespace router
{

    void TransportRouter::SetRoutingSettings(RoutingSettings routing_settings_)
    {
        routing_settings = std::move(routing_settings_);
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const
    {
        return routing_settings;
    }

    void TransportRouter::BuildRouter(TransportCatalogue& transport_catalogue_)
    {
        SetGraph(transport_catalogue_);
        router = std::make_unique<Router<double>>(*graph);
        router->Build();
    }

    const DirectedWeightedGraph<double>& TransportRouter::GetGraph() const
    {
        return *graph;
    }

    const Router<double>& TransportRouter::GetRouter() const
    {
        return *router;
    }

    const std::variant<StopEdge, BusEdge>& TransportRouter::GetEdge(EdgeId id_) const
    {
        return edge_id_to_edge.at(id_);
    }

    std::optional<StopVertexPair> TransportRouter::GetRouterByStop(Stop* stop_) const
    {
        if (stop_to_router.count(stop_))
        {
            return stop_to_router.at(stop_);
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<RouteInfo> TransportRouter::GetRouterInfo(VertexId start_, graph::VertexId end_) const
    {
        const auto& route_info = router->BuildRoute(start_, end_);
        if (route_info) 
        {
            RouteInfo result;
            result.total_time = route_info->weight;

            for (const auto edge : route_info->edges) 
            {
                result.edges.emplace_back(GetEdge(edge));
            }
            return result;
        }
        else 
        {
            return std::nullopt;
        }
    }

    const StopToRouter& TransportRouter::GetStopToVertex() const
    {
        return stop_to_router;
    }

    const EdgeIdToEdge& TransportRouter::GetEdgeIdToEdge() const
    {
        return edge_id_to_edge;
    }

    std::deque<Stop*> TransportRouter::GetStopsPtr(TransportCatalogue& transport_catalogue_)
    {
        std::deque<Stop*> stops_ptr;

        for (const auto& [_, stop_ptr] : transport_catalogue_.GetStopnameToStop())
        {
            stops_ptr.push_back(stop_ptr);
        }

        return stops_ptr;
    }

    std::deque<Bus*> TransportRouter::GetBusPtr(TransportCatalogue& transport_catalogue_)
    {
        std::deque<Bus*> buses_ptr;

        for (const auto& [_, bus_ptr] : transport_catalogue_.GetBusnameToBus())
        {
            buses_ptr.push_back(bus_ptr);
        }

        return buses_ptr;
    }

    void TransportRouter::SetStops(const std::deque<Stop*>& stops_)
    {
        size_t i = 0;

        for (const auto stop : stops_)
        {
            VertexId first = i++;
            VertexId second = i++;

            stop_to_router[stop] = StopVertexPair{ first, second };
        }
    }

    void TransportRouter::AddEdgeToStop()
    {
        for (const auto& [stop, num] : stop_to_router)
        {
            EdgeId id = graph->AddEdge(Edge<double>{num.bus_wait_start, num.bus_wait_end, routing_settings.bus_wait_time});

            edge_id_to_edge[id] = StopEdge{ stop->name, routing_settings.bus_wait_time };
        }
    }

    void TransportRouter::AddEdgeToBus(TransportCatalogue& transport_catalogue_)
    {
        for (auto bus : GetBusPtr(transport_catalogue_))
        {
            ParseBusToEdges(bus->stops.begin(), bus->stops.end(), transport_catalogue_, bus);

            if (!bus->is_roundtrip)
            {
                ParseBusToEdges(bus->stops.rbegin(), bus->stops.rend(), transport_catalogue_, bus);
            }
        }
    }

    void TransportRouter::SetGraph(TransportCatalogue& transport_catalogue_)
    {
        const auto stops_ptr_size = GetStopsPtr(transport_catalogue_).size();

        graph = std::make_unique<DirectedWeightedGraph<double>>(2 * stops_ptr_size);

        SetStops(GetStopsPtr(transport_catalogue_));
        AddEdgeToStop();
        AddEdgeToBus(transport_catalogue_);
    }

    Edge<double> TransportRouter::MakeEdgeToBus(Stop* start_, Stop* end_, const double distance_) const
    {
        Edge<double> result;

        result.from = stop_to_router.at(start_).bus_wait_end;
        result.to = stop_to_router.at(end_).bus_wait_start;
        result.weight = distance_ * 1.0 / (routing_settings.bus_velocity * KILOMETER / HOUR);

        return result;
    }

}//end namespace router
