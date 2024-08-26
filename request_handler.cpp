#include "request_handler.h"

namespace request_handler 
{
    struct EdgeInfoGetter
    {
        Node operator()(const StopEdge& edge_info_)
        {
            return Builder{}.StartDict()
                .Key("type"s).Value("Wait"s)
                .Key("stop_name"s).Value(std::string(edge_info_.stop_name))
                .Key("time"s).Value(edge_info_.time)
                .EndDict().Build();
        }

        Node operator()(const BusEdge& edge_info_)
        {
            return Builder{}.StartDict()
                .Key("type").Value("Bus")
                .Key("bus").Value(std::string(edge_info_.bus_name))
                .Key("span_count").Value(static_cast<int>(edge_info_.span_count))
                .Key("time").Value(edge_info_.time)
                .EndDict().Build();
        }
    };

    Node RequestHandler::ExecuteMakeNodeStop(int id_request_, const StopQueryResult& stop_info_) 
    {
        Node result;
        Array buses;
        Builder builder;

        std::string str_not_found = "not found";

        if (stop_info_.not_found) 
        {
            builder.StartDict()
                .Key("request_id").Value(id_request_)
                .Key("error_message").Value(str_not_found)
                .EndDict();

            result = builder.Build();

        }
        else 
        {
            builder.StartDict()
                .Key("request_id").Value(id_request_)
                .Key("buses").StartArray();

            for (const std::string& bus_name : stop_info_.buses_name) 
            {
                builder.Value(bus_name);
            }

            builder.EndArray().EndDict();

            result = builder.Build();
        }
        return result;
    }

    Node RequestHandler::ExecuteMakeNodeBus(int id_request_, const BusQueryResult& bus_info_) 
    {
        Node result;
        std::string str_not_found = "not found";

        if (bus_info_.not_found) 
        {
            result = Builder{}.StartDict()
                .Key("request_id").Value(id_request_)
                .Key("error_message").Value(str_not_found)
                .EndDict()
                .Build();
        }
        else 
        {
            result = Builder{}.StartDict()
                .Key("request_id").Value(id_request_)
                .Key("curvature").Value(bus_info_.curvature)
                .Key("route_length").Value(bus_info_.route_length)
                .Key("stop_count").Value(bus_info_.stops_on_route)
                .Key("unique_stop_count").Value(bus_info_.unique_stops)
                .EndDict()
                .Build();
        }
        return result;
    }

    Node RequestHandler::ExecuteMakeNodeMap(int id_request_, TransportCatalogue& catalogue_, RenderSettings render_settings_) 
    {
        Node result;

        std::ostringstream map_stream;
        std::string map_str;

        MapRenderer map_catalogue(render_settings_);

        map_catalogue.InitSphereProjector(catalogue_.GetStopCoordinates());

        ExecuteRenderMap(map_catalogue, catalogue_);
        map_catalogue.GetStreamMap(map_stream);
        map_str = map_stream.str();

        result = Builder{}
            .StartDict()
            .Key("request_id"s).Value(id_request_)
            .Key("map"s).Value(map_str)
            .EndDict().Build();

        return result;
    }

    Node RequestHandler::ExecuteMakeNodeRoute(StatRequest& request_, TransportCatalogue& catalogue_, TransportRouter& routing_) 
    {
        const auto& route_info = GetRouteInfo(request_.from, request_.to, catalogue_, routing_);

        if (!route_info) 
        {
            return Builder{}.StartDict()
                .Key("request_id").Value(request_.id)
                .Key("error_message").Value("not found")
                .EndDict()
                .Build();
        }

        Array items;
        for (const auto& item : route_info->edges) 
        {
            items.emplace_back(std::visit(EdgeInfoGetter{}, item));
        }

        return Builder{}.StartDict()
            .Key("request_id").Value(request_.id)
            .Key("total_time").Value(route_info->total_time)
            .Key("items").Value(items)
            .EndDict()
            .Build();
    }

    void RequestHandler::ExecuteQueries(TransportCatalogue& catalogue_, std::vector<StatRequest>& stat_requests_, RenderSettings& render_settings_, RoutingSettings& routing_settings_)
    {
        std::vector<Node> result_request;
        TransportRouter routing(catalogue_, routing_settings_);


        for (StatRequest req : stat_requests_) 
        {
            if (req.type == "Stop") 
            {
                result_request.push_back(ExecuteMakeNodeStop(req.id, StopQuery(catalogue_, req.name)));
            }
            else if (req.type == "Bus") 
            {
                result_request.push_back(ExecuteMakeNodeBus(req.id, BusQuery(catalogue_, req.name)));
            }
            else if (req.type == "Map") 
            {
                result_request.push_back(ExecuteMakeNodeMap(req.id, catalogue_, render_settings_));
            }
            else if (req.type == "Route") 
            {
                result_request.push_back(ExecuteMakeNodeRoute(req, catalogue_, routing));
            }
        }

        document = Document{ Node(result_request) };
    }

    void RequestHandler::ExecuteRenderMap(MapRenderer& map_catalogue_, TransportCatalogue& catalogue_) const 
    {
        std::vector<std::pair<Bus*, int>> buses_palette;
        std::vector<Stop*> stops_sort;
        int palette_size = 0;
        int palette_index = 0;

        palette_size = map_catalogue_.GetPaletteSize();

        if (palette_size == 0)
        {
            std::cout << "color palette is empty";
            return;
        }

        BusMap buses = catalogue_.GetBusnameToBus();
        if (buses.size() > 0) 
        {
            for (std::string_view bus_name : catalogue_.GetSortedBusesNames()) 
            {
                Bus* bus_info = catalogue_.GetBus(bus_name);

                if (bus_info) 
                {
                    if (bus_info->stops.size() > 0) 
                    {
                        buses_palette.push_back(std::make_pair(bus_info, palette_index));
                        palette_index++;

                        if (palette_index == palette_size) 
                        {
                            palette_index = 0;
                        }
                    }
                }
            }

            if (buses_palette.size() > 0) 
            {
                map_catalogue_.AddLine(buses_palette);
                map_catalogue_.AddBusesName(buses_palette);
            }
        }

        auto stops = catalogue_.GetStopnameToStop();
        if (stops.size() > 0) 
        {
            std::vector<std::string_view> stops_name;

            for (auto& [stop_name, stop] : stops) 
            {
                if (stop->buses.size() > 0) 
                {
                    stops_name.push_back(stop_name);
                }
            }

            std::sort(stops_name.begin(), stops_name.end());

            for (const std::string_view stop_name : stops_name) 
            {
                Stop* stop = catalogue_.GetStop(stop_name);
                if (stop) 
                {
                    stops_sort.push_back(stop);
                }
            }

            if (stops_sort.size() > 0) 
            {
                map_catalogue_.AddStopsCircle(stops_sort);
                map_catalogue_.AddStopsName(stops_sort);
            }
        }
    }

    std::optional<RouteInfo> RequestHandler::GetRouteInfo(std::string_view start_, std::string_view end_, TransportCatalogue& catalogue_, TransportRouter& routing_) const 
    {
        return routing_.GetRouterInfo(routing_.GetRouterByStop(catalogue_.GetStop(start_))->bus_wait_start, routing_.GetRouterByStop(catalogue_.GetStop(end_))->bus_wait_start);

    }

    BusQueryResult RequestHandler::BusQuery(TransportCatalogue& catalogue_, std::string_view bus_name_) 
    {
        BusQueryResult bus_info;
        Bus* bus = catalogue_.GetBus(bus_name_);

        if (bus != nullptr) 
        {
            bus_info.name = bus->name;
            bus_info.not_found = false;
            bus_info.stops_on_route = static_cast<int>(bus->stops.size());
            bus_info.unique_stops = static_cast<int>(catalogue_.GetUniqStops(bus).size());
            bus_info.route_length = static_cast<int>(bus->route_length);
            bus_info.curvature = double(catalogue_.GetDistanceToBus(bus) / catalogue_.GetLength(bus));
        }
        else 
        {
            bus_info.name = bus_name_;
            bus_info.not_found = true;
        }
        return bus_info;
    }

    StopQueryResult RequestHandler::StopQuery(TransportCatalogue& catalogue_, std::string_view stop_name_) 
    {
        std::unordered_set<const Bus*> unique_buses;
        StopQueryResult stop_info;
        Stop* stop = catalogue_.GetStop(stop_name_);

        if (stop != nullptr) 
        {
            stop_info.name = stop->name;
            stop_info.not_found = false;
            unique_buses = catalogue_.StopGetUniqBuses(stop);

            if (unique_buses.size() > 0) 
            {
                for (const Bus* bus : unique_buses) 
                {
                    stop_info.buses_name.push_back(bus->name);
                }

                std::sort(stop_info.buses_name.begin(), stop_info.buses_name.end());
            }
        }
        else 
        {
            stop_info.name = stop_name_;
            stop_info.not_found = true;
        }
        return stop_info;
    }

    const Document& RequestHandler::GetDocument() 
    {
        return document;
    }

}//end namespace request_handler