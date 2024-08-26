#include "svg.h"

namespace svg
{
    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out_, Color& color_)
    {
        std::visit(ColorPrinter{ out_ }, color_);
        return out_;
    }
    std::ostream& operator<<(std::ostream& out_, StrokeLineCap line_cap_)
    {
        switch (line_cap_)
        {
        case StrokeLineCap::BUTT:
            out_ << "butt"s;
            break;

        case StrokeLineCap::ROUND:
            out_ << "round"s;
            break;

        case StrokeLineCap::SQUARE:
            out_ << "square"s;
            break;
        }
        return out_;
    }

    std::ostream& operator<<(std::ostream& out_, StrokeLineJoin line_join_)
    {
        switch (line_join_)
        {
        case StrokeLineJoin::ARCS:
            out_ << "arcs"s;
            break;

        case StrokeLineJoin::BEVEL:
            out_ << "bevel"s;
            break;

        case StrokeLineJoin::MITER:
            out_ << "miter"s;
            break;

        case StrokeLineJoin::MITER_CLIP:
            out_ << "miter-clip"s;
            break;

        case StrokeLineJoin::ROUND:
            out_ << "round"s;
            break;
        }
        return out_;
    }

    void Object::Render(const RenderContext& context_) const
    {
        context_.RenderIndent();
        RenderObject(context_);
        context_.out << std::endl;
    }

    Circle& Circle::SetCenter(Point center_)
    {
        center = center_;
        return *this;
    }

    Circle& Circle::SetRadius(double radius_)
    {
        radius = radius_;
        return *this;
    }

    Polyline& Polyline::AddPoint(Point point_)
    {
        points.push_back(std::move(point_));
        return *this;
    }

    Text& Text::SetPosition(Point pos_)
    {
        pos = pos_;
        return *this;
    }

    Text& Text::SetOffset(Point offset_)
    {
        offset = offset_;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size_)
    {
        size = size_;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family_)
    {
        font_family = std::move(font_family_);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight_)
    {
        font_weight = std::move(font_weight_);
        return *this;
    }

    Text& Text::SetData(std::string data_)
    {
        data = std::move(data_);
        return *this;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj_)
    {
        objects.emplace_back(std::move(obj_));
    }

    void Document::Render(std::ostream& out_) const
    {
        RenderContext ctx(out_, 2, 2);

        out_ << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"s << std::endl;
        out_ << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"s << std::endl;
        for (const std::unique_ptr<Object>& obj : objects)
        {
            obj->Render(ctx);
        }
        out_ << "</svg>"s;
    }

}  // namespace svg