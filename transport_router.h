#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "domain.h"

#include <deque>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <memory>

namespace router
{
    using transport::TransportCatalogue;
    using namespace graph;

    static const uint16_t KILOMETER = 1000;
    static const uint16_t HOUR = 60;

    typedef std::unordered_map<Stop*, StopVertexPair> StopToRouter;
    typedef std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> EdgeIdToEdge;

    class TransportRouter
    {
    public:

        TransportRouter(TransportCatalogue& catalogue_, RoutingSettings routing_settings_)
        {
            SetRoutingSettings(routing_settings_);
            BuildRouter(catalogue_);
        }

        const RoutingSettings& GetRoutingSettings() const;

        std::optional<StopVertexPair> GetRouterByStop(Stop* stop_) const;
        std::optional<RouteInfo> GetRouterInfo(VertexId start, graph::VertexId end) const;

    private:

        StopToRouter stop_to_router;
        EdgeIdToEdge edge_id_to_edge;

        std::unique_ptr<DirectedWeightedGraph<double>> graph;
        std::unique_ptr<Router<double>> router;

        RoutingSettings routing_settings;

        Edge<double> MakeEdgeToBus(Stop* start_, Stop* end_, const double distance_) const;

        const DirectedWeightedGraph<double>& GetGraph() const;
        const Router<double>& GetRouter() const;
        const std::variant<StopEdge, BusEdge>& GetEdge(EdgeId id_) const;

        const StopToRouter& GetStopToVertex() const;
        const EdgeIdToEdge& GetEdgeIdToEdge() const;

        std::deque<Stop*> GetStopsPtr(TransportCatalogue& transport_catalogue_);
        std::deque<Bus*> GetBusPtr(TransportCatalogue& transport_catalogue_);

        void AddEdgeToStop();
        void AddEdgeToBus(TransportCatalogue& transport_catalogue_);

        void SetStops(const std::deque<Stop*>& stops_);
        void SetGraph(TransportCatalogue& transport_catalogue_);

        void SetRoutingSettings(RoutingSettings routing_settings_);
        void BuildRouter(TransportCatalogue& transport_catalogue_);

        template <typename Iterator>
        void ParseBusToEdges(Iterator first_, Iterator last_, const TransportCatalogue& transport_catalogue_, const Bus* bus_)
        {
            for (auto it = first_; it != last_; ++it)
            {
                size_t distance = 0;
                size_t span = 0;

                for (auto it2 = std::next(it); it2 != last_; ++it2)
                {
                    distance += transport_catalogue_.GetDistanceStop(*prev(it2), *it2);
                    ++span;

                    EdgeId id = graph->AddEdge(MakeEdgeToBus(*it, *it2, distance));

                    edge_id_to_edge[id] = BusEdge{ bus_->name, span, graph->GetEdge(id).weight };
                }
            }
        }
    };

}//end namespace router