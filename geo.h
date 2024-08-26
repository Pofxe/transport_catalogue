#pragma once

#include <cmath>
#include <stdexcept>

namespace geo
{
    struct Coordinates
    {
        double latitude = 0.0;
        double longitude = 0.0;

        bool operator==(const Coordinates& other) const
        {
            const double epsilon = 1e-07;

            return std::abs(latitude - other.latitude) < epsilon && std::abs(longitude - other.longitude) < epsilon;
        }

        bool operator!=(const Coordinates& other) const
        {
            return !(*this == other);
        }
    };

    inline double ComputeDistance(Coordinates from, Coordinates to)
    {
        using namespace std::string_literals;

        if (from == to)
        {
            return 0.0;
        }
        if ((from.latitude < -90 || from.latitude > 90) && (from.longitude < -180 || from.longitude > 180))
        {
            throw std::invalid_argument("The coordinates were not transmitted correctly"s);
        }
        if ((to.latitude < -90 || to.latitude > 90) && (to.longitude < -180 || to.longitude > 180))
        {
            throw std::invalid_argument("The coordinates were not transmitted correctly"s);
        }

        static const double Pi = 3.1415926535;
        static const double dr = Pi / 180.;
        static const int earth_rd = 6371000;

        return acos(sin(from.latitude * dr) * sin(to.latitude * dr) + cos(from.latitude * dr) * cos(to.latitude * dr) * cos(abs(from.longitude - to.longitude) * dr)) * earth_rd;
    }

} // namespace geo