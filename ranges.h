#pragma once

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace ranges
{
    template <typename Iter>
    class Range
    {
    public:

        using ValueType = typename std::iterator_traits<Iter>::value_type;

        Range(Iter begin_, Iter end_) : begin_iter(begin_), end_iter(end_) {}

        Iter begin() const
        {
            return begin_iter;
        }
        Iter end() const
        {
            return end_iter;
        }

    private:

        Iter begin_iter;
        Iter end_iter;
    };

    template <typename C>
    auto AsRange(const C& container_)
    {
        return Range{ container_.begin(), container_.end() };
    }

}//end namespace ranges