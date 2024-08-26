#pragma once

#include "geo.h"
#include "graph.h"

#include <algorithm>
#include <vector>
#include <string>
#include <variant>

struct StatRequest
{
    int id = 0;

    std::string type;
    std::string name;

    std::string from;
    std::string to;
};

struct Bus;
struct Stop
{
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
    std::vector<Bus*> buses;
};

struct Bus
{
    std::string name;
    bool is_roundtrip = false;
    size_t route_length = 0;
    std::vector<Stop*> stops;
};

struct Distance
{
    const Stop* start;
    const Stop* end;
    int distance = 0;
};

struct StopDistancesHasher
{
    size_t operator()(const std::pair<const Stop*, const Stop*>& points_) const
    {
        size_t hash_first = std::hash<const void*>{}(points_.first);
        size_t hash_second = std::hash<const void*>{}(points_.second);
        return hash_first ^ (hash_second + 0x9e3779b9 + (hash_first << 6) + (hash_first >> 2));
    }
};

struct BusQueryResult
{
    std::string_view name;
    bool not_found = false;
    int stops_on_route = 0;
    int unique_stops = 0;
    int route_length = 0;
    double curvature = 0.0;
};

struct StopQueryResult
{
    std::string_view name;
    bool not_found = false;
    std::vector <std::string> buses_name;
};

struct BusEdge
{
    std::string_view bus_name;
    size_t span_count = 0;
    double time = 0;
};

struct StopEdge
{
    std::string_view stop_name;
    double time = 0;
};

struct StopVertexPair
{
    graph::VertexId bus_wait_start;
    graph::VertexId bus_wait_end;
};

struct RoutingSettings
{
    double bus_wait_time = 0;
    double bus_velocity = 0;
};

struct RouteInfo
{
    double total_time = 0.;
    std::vector<std::variant<StopEdge, BusEdge>> edges;
};