#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <iostream>
#include <optional>
#include <algorithm>
#include <cstdlib>

namespace map_renderer
{
    using namespace std::literals;

    inline const double EPSILON = 1e-6;

    class SphereProjector
    {
    public:

        SphereProjector() = default;


        template <typename InputIt>
        SphereProjector(InputIt points_begin_, InputIt points_end_, double max_width_, double max_height_, double padding_);

        svg::Point operator()(geo::Coordinates coordinates_) const;

    private:

        double padding = 0.0;
        double min_lon = 0.0;
        double max_lat = 0.0;
        double zoom_coeff = 0.0;

        bool IsZero(double value_);
    };

    struct RenderSettings
    {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;

        std::pair<double, double> bus_label_offset;
        int stop_label_font_size = 0;
        std::pair<double, double> stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer
    {
    public:

        MapRenderer(RenderSettings& render_settings_);

        SphereProjector GetSphereProjector(const std::vector<geo::Coordinates>& points_) const;
        void InitSphereProjector(std::vector<geo::Coordinates> points_);

        RenderSettings GetRenderSettings() const;
        int GetPaletteSize() const;
        svg::Color GetColor(int line_number_) const;

        void AddLine(std::vector<std::pair<Bus*, int>>& buses_palette_);
        void AddBusesName(std::vector<std::pair<Bus*, int>>& buses_palette_);
        void AddStopsCircle(std::vector<Stop*>& stops_name_);
        void AddStopsName(std::vector<Stop*>& stops_name_);

        void GetStreamMap(std::ostream& stream_);

    private:

        SphereProjector sphere_projector;
        RenderSettings& render_settings;
        svg::Document map_svg; // svg::

        void SetLineProperties(svg::Polyline& polyline_, int line_number_) const;

        void SetRouteTextCommonProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const;
        void SetRouteTextAdditionalProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const;
        void SetRouteTextColorProperties(svg::Text& text_, const std::string& name_, int palette_, svg::Point position_) const;

        void SetStopsCirclesProperties(svg::Circle& circle_, svg::Point position_) const;
        void SetStopsTextCommonProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const;
        void SetStopsTextAdditionalProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const;
        void SetStopsTextColorProperties(svg::Text& text_, const std::string& name_, svg::Point position_) const;
    };

    template <typename InputIt>
    SphereProjector::SphereProjector(InputIt points_begin_, InputIt points_end_, double max_width_, double max_height_, double padding_) : padding(padding_)
    {
        if (points_begin_ == points_end_)
        {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(points_begin_, points_end_, [](auto lhs, auto rhs)
            {
                return lhs.longitude < rhs.longitude;
            });

        min_lon = left_it->longitude;
        const double max_lon = right_it->longitude;

        const auto [bottom_it, top_it] = std::minmax_element(points_begin_, points_end_, [](auto lhs, auto rhs)
            {
                return lhs.latitude < rhs.latitude;
            });

        const double min_lat = bottom_it->latitude;
        max_lat = top_it->latitude;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon))
        {
            width_zoom = (max_width_ - 2 * padding_) / (max_lon - min_lon);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat - min_lat))
        {
            height_zoom = (max_height_ - 2 * padding_) / (max_lat - min_lat);
        }

        if (width_zoom && height_zoom)
        {
            zoom_coeff = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom)
        {
            zoom_coeff = *width_zoom;
        }
        else if (height_zoom)
        {
            zoom_coeff = *height_zoom;
        }
    }

} // end namespace map_renderer