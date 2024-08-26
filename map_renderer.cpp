#include "map_renderer.h"

namespace map_renderer
{
    bool SphereProjector::IsZero(double value_)
    {
        return std::abs(value_) < EPSILON;
    }

    MapRenderer::MapRenderer(RenderSettings& render_settings_) : render_settings(render_settings_) {}

    svg::Point SphereProjector::operator()(geo::Coordinates coordinates_) const
    {
        return { (coordinates_.longitude - min_lon) * zoom_coeff + padding, (max_lat - coordinates_.latitude) * zoom_coeff + padding };
    }

    SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates>& points_) const
    {
        return SphereProjector(points_.begin(), points_.end(), render_settings.width, render_settings.height, render_settings.padding);
    }

    void MapRenderer::InitSphereProjector(std::vector<geo::Coordinates> points_)
    {
        sphere_projector = SphereProjector(points_.begin(), points_.end(), render_settings.width, render_settings.height, render_settings.padding);
    }

    RenderSettings MapRenderer::GetRenderSettings() const
    {
        return render_settings;
    }

    int MapRenderer::GetPaletteSize() const
    {
        return render_settings.color_palette.size();
    }

    svg::Color MapRenderer::GetColor(int line_number_) const
    {
        return render_settings.color_palette[line_number_];
    }

    void MapRenderer::SetLineProperties(svg::Polyline& polyline_, [[maybe_unused]] int line_number_) const
    {
        polyline_.SetStrokeColor(GetColor(line_number_));
        polyline_.SetFillColor("none"s);
        polyline_.SetStrokeWidth(render_settings.line_width);
        polyline_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    void MapRenderer::SetRouteTextCommonProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const
    {

        text_.SetPosition(position_);
        text_.SetOffset({ render_settings.bus_label_offset.first, render_settings.bus_label_offset.second });
        text_.SetFontSize(render_settings.bus_label_font_size);
        text_.SetFontFamily("Verdana"s);
        text_.SetFontWeight("bold"s);
        text_.SetData(name_);
    }

    void MapRenderer::SetRouteTextAdditionalProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const
    {
        SetRouteTextCommonProperties(text_, name_, position_);

        text_.SetFillColor(render_settings.underlayer_color);
        text_.SetStrokeColor(render_settings.underlayer_color);
        text_.SetStrokeWidth(render_settings.underlayer_width);
        text_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    }

    void MapRenderer::SetRouteTextColorProperties(svg::Text& text_, const std::string& name_, int palette_, svg::Point position_) const
    {
        SetRouteTextCommonProperties(text_, name_, position_);

        text_.SetFillColor(GetColor(palette_));
    }

    void MapRenderer::SetStopsCirclesProperties(svg::Circle& circle_, svg::Point position_) const
    {
        circle_.SetCenter(position_);
        circle_.SetRadius(render_settings.stop_radius);
        circle_.SetFillColor("white"s);
    }

    void MapRenderer::SetStopsTextCommonProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const
    {
        text_.SetPosition(position_);
        text_.SetOffset({ render_settings.stop_label_offset.first, render_settings.stop_label_offset.second });
        text_.SetFontSize(render_settings.stop_label_font_size);
        text_.SetFontFamily("Verdana"s);
        text_.SetData(name_);
    }

    void MapRenderer::SetStopsTextAdditionalProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const
    {
        SetStopsTextCommonProperties(text_, name_, position_);

        text_.SetFillColor(render_settings.underlayer_color);
        text_.SetStrokeColor(render_settings.underlayer_color);
        text_.SetStrokeWidth(render_settings.underlayer_width);
        text_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    }

    void MapRenderer::SetStopsTextColorProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const
    {
        SetStopsTextCommonProperties(text_, name_, position_);
        text_.SetFillColor("black"s);
    }

    void MapRenderer::AddLine(std::vector<std::pair<Bus*, int>>& buses_palette_)
    {
        std::vector<geo::Coordinates> stops_geo_coords;

        for (const auto& [bus, palette] : buses_palette_)
        {

            for (const Stop* stop : bus->stops)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop->latitude;
                coordinates.longitude = stop->longitude;

                stops_geo_coords.push_back(coordinates);
            }

            svg::Polyline bus_line;
            bool bus_empty = true;

            for (const auto& coord : stops_geo_coords)
            {
                bus_empty = false;
                bus_line.AddPoint(sphere_projector(coord));
            }
            if (!bus_empty)
            {
                SetLineProperties(bus_line, palette);
                map_svg.Add(bus_line);
            }
            stops_geo_coords.clear();
        }
    }

    void MapRenderer::AddBusesName(std::vector<std::pair<Bus*, int>>& buses_palette_)
    {
        std::vector<geo::Coordinates> stops_geo_coords;
        bool bus_empty = true;

        for (const auto& [bus, palette] : buses_palette_)
        {

            for (const Stop* stop : bus->stops)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop->latitude;
                coordinates.longitude = stop->longitude;

                stops_geo_coords.push_back(coordinates);

                if (bus_empty)
                {
                    bus_empty = false;
                }
            }

            svg::Text route_name_roundtrip;
            svg::Text route_title_roundtrip;
            svg::Text route_name_notroundtrip;
            svg::Text route_title_notroundtrip;

            if (!bus_empty)
            {
                if (bus->is_roundtrip)
                {
                    SetRouteTextAdditionalProperties(route_name_roundtrip, std::string(bus->name), sphere_projector(stops_geo_coords[0]));
                    map_svg.Add(route_name_roundtrip);

                    SetRouteTextColorProperties(route_title_roundtrip, std::string(bus->name), palette, sphere_projector(stops_geo_coords[0]));
                    map_svg.Add(route_title_roundtrip);

                }
                else
                {
                    SetRouteTextAdditionalProperties(route_name_roundtrip, std::string(bus->name), sphere_projector(stops_geo_coords[0]));
                    map_svg.Add(route_name_roundtrip);

                    SetRouteTextColorProperties(route_title_roundtrip, std::string(bus->name), palette, sphere_projector(stops_geo_coords[0]));
                    map_svg.Add(route_title_roundtrip);

                    if (stops_geo_coords[0] != stops_geo_coords[stops_geo_coords.size() / 2])
                    {
                        SetRouteTextAdditionalProperties(route_name_notroundtrip, std::string(bus->name), sphere_projector(stops_geo_coords[stops_geo_coords.size() / 2]));
                        map_svg.Add(route_name_notroundtrip);

                        SetRouteTextColorProperties(route_title_notroundtrip, std::string(bus->name), palette, sphere_projector(stops_geo_coords[stops_geo_coords.size() / 2]));
                        map_svg.Add(route_title_notroundtrip);
                    }
                }
            }
            bus_empty = false;
            stops_geo_coords.clear();
        }
    }

    void MapRenderer::AddStopsCircle(std::vector<Stop*>& stops_)
    {
        std::vector<geo::Coordinates> stops_geo_coords;
        svg::Circle icon;

        for (const Stop* stop_info : stops_)
        {
            if (stop_info)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop_info->latitude;
                coordinates.longitude = stop_info->longitude;

                SetStopsCirclesProperties(icon, sphere_projector(coordinates));
                map_svg.Add(icon);
            }
        }
    }

    void MapRenderer::AddStopsName(std::vector<Stop*>& stops_)
    {
        std::vector<geo::Coordinates> stops_geo_coords;

        svg::Text svg_stop_name;
        svg::Text svg_stop_name_title;

        for (const Stop* stop_info : stops_)
        {
            if (stop_info)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop_info->latitude;
                coordinates.longitude = stop_info->longitude;

                SetStopsTextAdditionalProperties(svg_stop_name, stop_info->name, sphere_projector(coordinates));
                map_svg.Add(svg_stop_name);

                SetStopsTextColorProperties(svg_stop_name_title, stop_info->name, sphere_projector(coordinates));
                map_svg.Add(svg_stop_name_title);
            }
        }
    }

    void MapRenderer::GetStreamMap(std::ostream& stream_)
    {
        map_svg.Render(stream_);
    }

}//end namespace map_renderer