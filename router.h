#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graph
{
    template <typename Weight>
    class Router
    {
        using Graph = DirectedWeightedGraph<Weight>;

    public:

        explicit Router(const Graph& graph_);

        struct RouteInfo
        {
            Weight weight;
            std::vector<EdgeId> edges;
        };

        void Build()
        {
            InitializeRoutesInternalData(graph);
            const size_t vertex_count = graph.GetVertexCount();

            for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through)
            {
                RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
            }
        }

        std::optional<RouteInfo> BuildRoute(VertexId from_, VertexId to_) const;

    private:

        struct RouteInternalData
        {
            Weight weight;
            std::optional<EdgeId> prev_edge;
        };

        using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

        void InitializeRoutesInternalData(const Graph& graph_)
        {
            using namespace std::string_literals;

            const size_t vertex_count = graph_.GetVertexCount();

            for (VertexId vertex = 0; vertex < vertex_count; ++vertex)
            {
                routes_internal_data[vertex][vertex] = RouteInternalData{ ZERO_WEIGHT, std::nullopt };

                for (const EdgeId edge_id : graph_.GetIncidentEdges(vertex))
                {
                    const auto& edge = graph_.GetEdge(edge_id);

                    if (edge.weight < ZERO_WEIGHT)
                    {
                        throw std::domain_error("Edges' weights should be non-negative"s);
                    }

                    auto& route_internal_data = routes_internal_data[vertex][edge.to];

                    if (!route_internal_data || route_internal_data->weight > edge.weight)
                    {
                        route_internal_data = RouteInternalData{ edge.weight, edge_id };
                    }
                }
            }
        }

        void RelaxRoute(VertexId vertex_from_, VertexId vertex_to_, const RouteInternalData& route_from_, const RouteInternalData& route_to_)
        {
            auto& route_relaxing = routes_internal_data[vertex_from_][vertex_to_];
            const Weight candidate_weight = route_from_.weight + route_to_.weight;

            if (!route_relaxing || candidate_weight < route_relaxing->weight)
            {
                route_relaxing = { candidate_weight, route_to_.prev_edge ? route_to_.prev_edge : route_from_.prev_edge };
            }
        }

        void RelaxRoutesInternalDataThroughVertex(size_t vertex_count_, VertexId vertex_through_)
        {
            for (VertexId vertex_from = 0; vertex_from < vertex_count_; ++vertex_from)
            {
                if (const auto& route_from = routes_internal_data[vertex_from][vertex_through_])
                {
                    for (VertexId vertex_to = 0; vertex_to < vertex_count_; ++vertex_to)
                    {
                        if (const auto& route_to = routes_internal_data[vertex_through_][vertex_to])
                        {
                            RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                        }
                    }
                }
            }
        }

        static constexpr Weight ZERO_WEIGHT{};
        const Graph& graph;
        RoutesInternalData routes_internal_data;
    };

    template <typename Weight>
    Router<Weight>::Router(const Graph& graph_) : graph(graph_), routes_internal_data(graph_.GetVertexCount(), std::vector<std::optional<RouteInternalData>>(graph_.GetVertexCount()))
    {
        InitializeRoutesInternalData(graph_);

        const size_t vertex_count = graph_.GetVertexCount();

        for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through)
        {
            RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
        }
    }

    template <typename Weight>
    std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from_, VertexId to_) const
    {
        const auto& route_internal_data = routes_internal_data.at(from_).at(to_);

        if (!route_internal_data)
        {
            return std::nullopt;
        }

        const Weight weight = route_internal_data->weight;
        std::vector<EdgeId> edges;

        for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge; edge_id; edge_id = routes_internal_data[from_][graph.GetEdge(*edge_id).from]->prev_edge)
        {
            edges.push_back(*edge_id);
        }

        std::reverse(edges.begin(), edges.end());

        return RouteInfo{ weight, std::move(edges) };
    }

}//end namespace graph