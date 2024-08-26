#include "json_reader.h"

namespace json
{
    JSON_R::JSON_R(Document doc_) : document(std::move(doc_)) {}
    JSON_R::JSON_R(std::istream& input_) : document(json::Load(input_)) {}

    Stop JSON_R::ParseNodeStop(const Node& node_)
    {
        Stop stop;
        Dict stop_node;

        if (node_.IsDict())
        {
            stop_node = node_.AsDict();
            stop.name = stop_node.at("name"s).AsString();
            stop.latitude = stop_node.at("latitude"s).AsDouble();
            stop.longitude = stop_node.at("longitude"s).AsDouble();
        }
        return stop;
    }

    std::vector<Distance> JSON_R::ParseNodeDistance(const Node& node_, TransportCatalogue& catalogue_)
    {
        std::vector<Distance> distances;
        Dict stop_node;
        Dict stop_road_map;
        std::string begin_name;
        std::string last_name;
        int distance = 0;

        if (node_.IsDict())
        {
            stop_node = node_.AsDict();
            begin_name = stop_node.at("name"s).AsString();

            try
            {
                stop_road_map = stop_node.at("road_distances"s).AsDict();

                for (const auto& [key, value] : stop_road_map)
                {
                    last_name = key;
                    distance = value.AsInt();
                    distances.push_back({ catalogue_.GetStop(begin_name), catalogue_.GetStop(last_name), distance });
                }
            }
            catch (...)
            {
                std::cout << "invalide road"s << std::endl;
            }
        }
        return distances;
    }

    Bus JSON_R::ParseNodeBus(const Node& node_, TransportCatalogue& catalogue_)
    {
        Bus bus;
        Dict bus_node;
        Array bus_stops;

        if (node_.IsDict())
        {
            bus_node = node_.AsDict();
            bus.name = bus_node.at("name"s).AsString();
            bus.is_roundtrip = bus_node.at("is_roundtrip"s).AsBool();

            try
            {
                bus_stops = bus_node.at("stops"s).AsArray();

                for (const Node& stop : bus_stops)
                {
                    bus.stops.push_back(catalogue_.GetStop(stop.AsString()));
                }

                if (!bus.is_roundtrip)
                {
                    size_t size = bus.stops.size() - 1;

                    for (size_t i = size; i > 0; i--)
                    {
                        bus.stops.push_back(bus.stops[i - 1]);
                    }

                }

            }
            catch (...)
            {
                std::cout << "base_requests: bus: stops is empty"s << std::endl;
            }
        }
        return bus;
    }

    void JSON_R::ParseNodeBase(const Node& root_, TransportCatalogue& catalogue_)
    {
        Array base_requests;
        Dict req_map;
        Node req_node;

        std::vector<Node> buses;
        std::vector<Node> stops;

        if (root_.IsArray())
        {
            base_requests = root_.AsArray();

            for (const Node& node : base_requests)
            {
                if (node.IsDict())
                {
                    req_map = node.AsDict();

                    try
                    {
                        req_node = req_map.at("type"s);

                        if (req_node.IsString())
                        {
                            if (req_node.AsString() == "Bus"s)
                            {
                                buses.push_back(req_map);
                            }
                            else if (req_node.AsString() == "Stop"s)
                            {
                                stops.push_back(req_map);
                            }
                            else
                            {
                                std::cout << "base_requests are invalid"s;
                            }
                        }
                    }
                    catch (...)
                    {
                        std::cout << "base_requests does not have type value"s;
                    }
                }
            }

            for (const Node& stop : stops)
            {
                catalogue_.AddStop(ParseNodeStop(stop));
            }
            for (const Node& stop : stops)
            {
                catalogue_.AddDistance(ParseNodeDistance(stop, catalogue_));
            }
            for (const Node& bus : buses)
            {
                catalogue_.AddBus(ParseNodeBus(bus, catalogue_));
            }
        }
        else
        {
            std::cout << "base_requests is not an array"s;
        }
    }

    void JSON_R::ParseNodeStat(const Node& node_, std::vector<StatRequest>& stat_request_)
    {
        Array stat_requests;
        Dict req_map;
        StatRequest req;

        if (node_.IsArray())
        {
            stat_requests = node_.AsArray();

            for (const Node& req_node : stat_requests)
            {
                if (req_node.IsDict())
                {
                    req_map = req_node.AsDict();
                    req.id = req_map.at("id"s).AsInt();
                    req.type = req_map.at("type"s).AsString();

                    if ((req.type == "Bus"s) || (req.type == "Stop"s))
                    {
                        req.name = req_map.at("name"s).AsString();
                        req.from = ""s;
                        req.to = ""s;
                    }
                    else
                    {
                        req.name = ""s;

                        if (req.type == "Route"s)
                        {
                            req.from = req_map.at("from"s).AsString();
                            req.to = req_map.at("to"s).AsString();
                        }
                        else
                        {
                            req.from = ""s;
                            req.to = ""s;
                        }
                    }
                    stat_request_.push_back(req);
                }
            }
        }
        else
        {
            std::cout << "base_requests is not array"s;
        }
    }

    void JSON_R::ParseNodeRender(const Node& node_, RenderSettings& rend_set_)
    {
        Dict rend_map;

        Array bus_lab_offset;
        Array stop_lab_offset;
        Array arr_color;
        Array arr_palette;

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 0.0;

        if (node_.IsDict())
        {
            rend_map = node_.AsDict();

            try
            {
                rend_set_.width = rend_map.at("width"s).AsDouble();
                rend_set_.height = rend_map.at("height"s).AsDouble();
                rend_set_.padding = rend_map.at("padding"s).AsDouble();
                rend_set_.line_width = rend_map.at("line_width"s).AsDouble();
                rend_set_.stop_radius = rend_map.at("stop_radius"s).AsDouble();

                rend_set_.bus_label_font_size = rend_map.at("bus_label_font_size"s).AsInt();

                if (rend_map.at("bus_label_offset"s).IsArray())
                {
                    bus_lab_offset = rend_map.at("bus_label_offset"s).AsArray();
                    rend_set_.bus_label_offset = std::make_pair(bus_lab_offset[0].AsDouble(), bus_lab_offset[1].AsDouble());
                }

                rend_set_.stop_label_font_size = rend_map.at("stop_label_font_size"s).AsInt();

                if (rend_map.at("stop_label_offset"s).IsArray())
                {
                    stop_lab_offset = rend_map.at("stop_label_offset"s).AsArray();
                    rend_set_.stop_label_offset = std::make_pair(stop_lab_offset[0].AsDouble(), stop_lab_offset[1].AsDouble());
                }
                if (rend_map.at("underlayer_color"s).IsString())
                {
                    rend_set_.underlayer_color = svg::Color(rend_map.at("underlayer_color"s).AsString());
                }
                else if (rend_map.at("underlayer_color"s).IsArray())
                {
                    arr_color = rend_map.at("underlayer_color"s).AsArray();
                    red = arr_color[0].AsInt();
                    green = arr_color[1].AsInt();
                    blue = arr_color[2].AsInt();

                    if (arr_color.size() == 4)
                    {
                        opacity = arr_color[3].AsDouble();
                        rend_set_.underlayer_color = svg::Color(svg::Rgba(red, green, blue, opacity));
                    }
                    else if (arr_color.size() == 3)
                    {
                        rend_set_.underlayer_color = svg::Color(svg::Rgb(red, green, blue));
                    }

                }

                rend_set_.underlayer_width = rend_map.at("underlayer_width"s).AsDouble();

                if (rend_map.at("color_palette"s).IsArray())
                {
                    arr_palette = rend_map.at("color_palette"s).AsArray();

                    for (const Node& color_palette : arr_palette)
                    {
                        if (color_palette.IsString())
                        {
                            rend_set_.color_palette.push_back(svg::Color(color_palette.AsString()));
                        }
                        else if (color_palette.IsArray())
                        {
                            arr_color = color_palette.AsArray();
                            red = arr_color[0].AsInt();
                            green = arr_color[1].AsInt();
                            blue = arr_color[2].AsInt();

                            if (arr_color.size() == 4)
                            {
                                opacity = arr_color[3].AsDouble();
                                rend_set_.color_palette.push_back(svg::Color(svg::Rgba(red, green, blue, opacity)));
                            }
                            else if (arr_color.size() == 3)
                            {
                                rend_set_.color_palette.push_back(svg::Color(svg::Rgb(red, green, blue)));
                            }
                        }
                    }
                }
            }
            catch (...)
            {
                std::cout << "unable to parsse init settings"s;
            }
        }
        else
        {
            std::cout << "render_settings is not map"s;
        }
    }

    void JSON_R::ParseNodeRouting(const Node& node_, RoutingSettings& route_set_)
    {
        Dict route;

        if (node_.IsDict())
        {
            route = node_.AsDict();

            try
            {
                route_set_.bus_wait_time = route.at("bus_wait_time"s).AsDouble();
                route_set_.bus_velocity = route.at("bus_velocity"s).AsDouble();
            }
            catch (...)
            {
                std::cout << "unable to parse routing settings"s;
            }
        }
        else
        {
            std::cout << "routing settings is not map"s;
        }
    }

    void JSON_R::ParseNode(const Node& root_, TransportCatalogue& catalogue_, std::vector<StatRequest>& stat_request_, RenderSettings& render_settings_, RoutingSettings& routing_settings_)
    {
        Dict root_dictionary;

        if (root_.IsDict())
        {
            root_dictionary = root_.AsDict();

            try
            {
                ParseNodeBase(root_dictionary.at("base_requests"s), catalogue_);
            }
            catch (...)
            {
                std::cout << "base_requests is empty"s;
            }

            try
            {
                ParseNodeStat(root_dictionary.at("stat_requests"s), stat_request_);
            }
            catch (...)
            {
                std::cout << "stat_requests is empty"s;
            }

            try
            {
                ParseNodeRender(root_dictionary.at("render_settings"s), render_settings_);
            }
            catch (...)
            {
                std::cout << "render_settings is empty"s;
            }

            try
            {
                ParseNodeRouting(root_dictionary.at("routing_settings"s), routing_settings_);
            }
            catch (...)
            {
                std::cout << "routing_settings is empty"s;
            }
        }
        else
        {
            std::cout << "root is not map"s;
        }
    }

    void JSON_R::Parse(TransportCatalogue& catalogue_, std::vector<StatRequest>& stat_request_, RenderSettings& render_settings_, RoutingSettings& routing_settings_)
    {
        ParseNode(document.GetRoot(), catalogue_, stat_request_, render_settings_, routing_settings_);
    }

    const Document& JSON_R::GetDocument() const
    {
        return document;
    }

} // end namespace json
