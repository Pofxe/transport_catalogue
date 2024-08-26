#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph
{
    using VertexId = size_t;
    using EdgeId = size_t;

    template <typename Weight>
    struct Edge
    {
        VertexId from = 0;
        VertexId to = 0;
        Weight weight = 0;
    };

    template <typename Weight>
    class DirectedWeightedGraph
    {
    private:

        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

    public:

        DirectedWeightedGraph() = default;
        explicit DirectedWeightedGraph(size_t vertex_count_);
        EdgeId AddEdge(const Edge<Weight>& edge_);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        const Edge<Weight>& GetEdge(EdgeId edge_id_) const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex_) const;

    private:

        std::vector<Edge<Weight>> edges;
        std::vector<IncidenceList> incidence_lists;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count_) : incidence_lists(vertex_count_) {}

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge_)
    {
        edges.push_back(edge_);

        const EdgeId id = edges.size() - 1;

        incidence_lists.at(edge_.from).push_back(id);

        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const
    {
        return incidence_lists.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const
    {
        return edges.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id_) const
    {
        return edges.at(edge_id_);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex_) const
    {
        return ranges::AsRange(incidence_lists.at(vertex_));
    }

}//end namespace graph