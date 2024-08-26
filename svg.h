#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg
{
    using namespace std::literals;

    struct Rgb
    {
        Rgb() : red(0), green(0), blue(0) {}

        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_) : red(red_), green(green_), blue(blue_) {}

        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct Rgba : public Rgb
    {
        Rgba() : Rgb(), opacity(1.0) {}

        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_) : Rgb(red_, green_, blue_), opacity(opacity_) {}

        double opacity = 0.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ std::monostate() };

    std::ostream& operator<<(std::ostream& out_, Color& color_);

    struct ColorPrinter
    {
        std::ostream& out;

        void operator()(std::monostate) const
        {
            out << "none"s;
        }

        void operator()(std::string color_) const
        {
            out << color_;
        }

        void operator()(Rgb color_) const
        {
            out << "rgb("s << static_cast<int>(color_.red) << ","s << static_cast<int>(color_.green) << ","s << static_cast<int>(color_.blue) << ")"s;
        }

        void operator()(Rgba color_) const
        {
            out << "rgba("s << static_cast<int>(color_.red) << ","s << static_cast<int>(color_.green) << ","s << static_cast<int>(color_.blue) << ","s << color_.opacity << ")"s;
        }
    };

    enum class StrokeLineCap { BUTT, ROUND, SQUARE, };

    enum class StrokeLineJoin { ARCS, BEVEL, MITER, MITER_CLIP, ROUND, };

    std::ostream& operator<<(std::ostream& out_, StrokeLineCap line_cap_);
    std::ostream& operator<<(std::ostream& out_, StrokeLineJoin line_join_);

    struct Point
    {
        Point() = default;
        Point(double x, double y) : x(x), y(y) {}

        double x = 0.0;
        double y = 0.0;
    };

    struct RenderContext
    {
        RenderContext(std::ostream& out_) : out(out_) {}
        RenderContext(std::ostream& out_, int indent_step_, int indent_ = 0) : out(out_), indent_step(indent_step_), indent(indent_) {}

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object
    {
    public:

        void Render(const RenderContext& context_) const;
        virtual ~Object() = default;

    private:

        virtual void RenderObject(const RenderContext& context_) const = 0;
    };

    class ObjectContainer
    {
    public:

        template <typename Type>
        void Add(Type obj_)
        {
            AddPtr(std::make_unique<Type>(std::move(obj_)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj_) = 0;

    protected:

        ~ObjectContainer() = default;
    };

    class Drawable
    {
    public:

        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container_) const = 0;
    };

    template <typename Owner>
    class PathProps
    {
    public:
        Owner& SetFillColor(Color color_)
        {
            fill_color = std::move(color_);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color_)
        {
            stroke_color = std::move(color_);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width_)
        {
            width = std::move(width_);
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap_)
        {
            line_cap = line_cap_;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join_)
        {
            line_join = line_join_;
            return AsOwner();
        }

    protected:

        ~PathProps() = default;

        void RenderAttrs(std::ostream& out_) const
        {
            if (fill_color)
            {
                out_ << " fill=\""s;
                std::visit(ColorPrinter{ out_ }, *fill_color);
                out_ << "\""sv;
            }
            if (stroke_color)
            {
                out_ << " stroke=\""s;
                std::visit(ColorPrinter{ out_ }, *stroke_color);
                out_ << "\""s;
            }
            if (width)
            {
                out_ << " stroke-width=\""s << *width << "\""s;
            }
            if (line_cap)
            {
                out_ << " stroke-linecap=\""s << *line_cap << "\""s;
            }
            if (line_join)
            {
                out_ << " stroke-linejoin=\""s << *line_join << "\""s;
            }
        }

    private:

        std::optional<Color> fill_color;
        std::optional<Color> stroke_color;

        std::optional<double> width;

        std::optional<StrokeLineCap> line_cap;
        std::optional<StrokeLineJoin> line_join;

        Owner& AsOwner()
        {
            return static_cast<Owner&>(*this);
        }
    };

    class Circle final : public Object, public PathProps<Circle>
    {
    public:

        Circle& SetCenter(Point center_);
        Circle& SetRadius(double radius_);

    private:

        Point center;
        double radius = 1.0;

        void RenderObject(const RenderContext& context_) const override
        {

            std::ostream& out = context_.out;

            out << "<circle cx=\""s << center.x << "\" cy=\""s << center.y << "\" "s;
            out << "r=\""s << radius << "\""s;

            RenderAttrs(context_.out);
            out << "/>"s;
        }
    };

    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:

        Polyline& AddPoint(Point point_);

    private:

        std::vector<Point> points;

        void RenderObject(const RenderContext& context_) const override
        {
            std::ostream& out = context_.out;
            out << "<polyline points=\""s;
            bool is_first = true;

            for (const svg::Point& point : points)
            {
                if (is_first)
                {
                    out << point.x << ","s << point.y;
                    is_first = false;
                }
                else
                {
                    out << " "s << point.x << ","s << point.y;
                }
            }
            out << "\""s;

            RenderAttrs(context_.out);
            out << "/>"s;
        }
    };

    class Text final : public Object, public PathProps<Text>
    {
    public:

        Text& SetPosition(Point pos_);
        Text& SetOffset(Point offset_);
        Text& SetFontSize(uint32_t size_);
        Text& SetFontFamily(std::string font_family_);
        Text& SetFontWeight(std::string font_weight_);
        Text& SetData(std::string data_);

    private:

        Point pos = { 0.0, 0.0 };
        Point offset = { 0.0, 0.0 };

        uint32_t size = 1;

        std::string font_family;
        std::string font_weight;
        std::string data;

        void RenderObject(const RenderContext& context_) const override
        {
            std::ostream& out = context_.out;
            out << "<text"s;

            RenderAttrs(context_.out);

            out << " x=\""s << pos.x << "\" y=\""s << pos.y << "\" "s;
            out << "dx=\""s << offset.x << "\" dy=\""s << offset.y << "\" "s;
            out << "font-size=\""s << size << "\""s;

            if (!font_family.empty())
            {
                out << " font-family=\""s << font_family << "\" "s;
            }
            if (!font_weight.empty())
            {
                out << "font-weight=\""s << font_weight << "\""s;
            }
            out << ">"s << data << "</text>"s;
        }
    };

    class Document : public ObjectContainer
    {
    public:

        void AddPtr(std::unique_ptr<Object>&& obj_) override;

        void Render(std::ostream& out_) const;

    private:

        std::vector<std::unique_ptr<Object>> objects;
    };

}  // namespace svg