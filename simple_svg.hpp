/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef SIMPLE_SVG_HPP
#define SIMPLE_SVG_HPP

#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace svg
{
    namespace {
        /**
         * Inserts a tab character at each line beginning of the string, i.e. after each line break except the last one
         * and at the beginning of the string.
         * Does not insert tab character, if the following line is empty.
         *
         * @param original Original string.
         * @return indented string.
         */
        std::string indent(std::string const & original) {
            std::string result;

            size_t next;
            size_t current = 0;
            do {
                next = original.find('\n', current);
                if (next != std::string::npos) ++next;
                std::string substr = original.substr(current, next - current);
                if(!substr.empty()) result.append("\t").append(substr);
                current = next;
            } while (next != std::string::npos && next != original.size());

            return result;
        }
    }

    // Utility XML/String Functions.
    template <typename T>
    inline std::string attribute(std::string const & attribute_name,
        T const & value, std::string const & unit = "")
    {
        std::stringstream ss;
        ss << attribute_name << "=\"" << value << unit << "\" ";
        return ss.str();
    }
    inline std::string elemStart(std::string const & element_name)
    {
        return "<" + element_name + " ";
    }
    inline std::string elemEnd(std::string const & element_name)
    {
        return "</" + element_name + ">\n";
    }
    inline std::string emptyElemEnd()
    {
        return "/>\n";
    }

    // Quick optional return type.  This allows functions to return an invalid
    //  value if no good return is possible.  The user checks for validity
    //  before using the returned value.
    template <typename T>
    class optional
    {
    public:
        explicit optional<T>(T const & type)
            : valid(true), type(type) { }
        optional<T>() : valid(false), type(T()) { }
        T * operator->()
        {
            // If we try to access an invalid value, an exception is thrown.
            if (!valid)
                throw std::exception();

            return &type;
        }
        // Test for validity.
        bool operator!() const { return !valid; }
    private:
        bool valid;
        T type;
    };

    struct Dimensions
    {
        Dimensions(double width, double height) : width(width), height(height) { }
        Dimensions(double combined = 0) : width(combined), height(combined) { }
        double width;
        double height;
    };

    struct Point
    {
        explicit Point(double x = 0, double y = 0) : x(x), y(y) { }
        double x;
        double y;
    };
    inline optional<Point> getMinPoint(std::vector<Point> const & points)
    {
        if (points.empty())
            return optional<Point>();

        Point min = points[0];
        for (auto & point : points) {
            if (point.x < min.x)
                min.x = point.x;
            if (point.y < min.y)
                min.y = point.y;
        }
        return optional<Point>(min);
    }
    inline optional<Point> getMaxPoint(std::vector<Point> const & points)
    {
        if (points.empty())
            return optional<Point>();

        Point max = points[0];
        for (auto & point : points) {
            if (point.x > max.x)
                max.x = point.x;
            if (point.y > max.y)
                max.y = point.y;
        }
        return optional<Point>(max);
    }

    // Defines the dimensions, scale, origin, and origin offset of the document.
    struct Layout
    {
        enum Origin { TopLeft, BottomLeft, TopRight, BottomRight };

        explicit Layout(Dimensions const & dimensions = Dimensions(400, 300), Origin origin = BottomLeft,
            double scale = 1, Point const & origin_offset = Point(0, 0))
            : dimensions(dimensions), scale(scale), origin(origin), origin_offset(origin_offset) { }
        Dimensions dimensions;
        double scale;
        Origin origin;
        Point origin_offset;
    };

    // Convert coordinates in user space to SVG native space.
    inline double translateX(double x, Layout const & layout)
    {
        if (layout.origin == Layout::BottomRight || layout.origin == Layout::TopRight)
            return layout.dimensions.width - ((x + layout.origin_offset.x) * layout.scale);
        else
            return (layout.origin_offset.x + x) * layout.scale;
    }

    inline double translateY(double y, Layout const & layout)
    {
        if (layout.origin == Layout::BottomLeft || layout.origin == Layout::BottomRight)
            return layout.dimensions.height - ((y + layout.origin_offset.y) * layout.scale);
        else
            return (layout.origin_offset.y + y) * layout.scale;
    }
    inline double translateScale(double const dimension, Layout const & layout)
    {
        return dimension * layout.scale;
    }

    class Serializable
    {
    public:
        Serializable() = default;
        virtual ~Serializable() = default;;
        virtual std::string toString(Layout const & layout) const = 0;
    };

    class Color : public Serializable
    {
    public:
        enum Defaults { Transparent = -1, Aqua, Black, Blue, Brown, Cyan, Fuchsia,
            Green, Lime, Magenta, Orange, Purple, Red, Silver, White, Yellow };

        Color(int r, int g, int b) : transparent(false), red(r), green(g), blue(b) { }
        Color(Defaults color)
            : transparent(false), red(0), green(0), blue(0)
        {
            switch (color)
            {
                case Aqua: assign(0, 255, 255); break;
                case Black: assign(0, 0, 0); break;
                case Blue: assign(0, 0, 255); break;
                case Brown: assign(165, 42, 42); break;
                case Cyan: assign(0, 255, 255); break;
                case Fuchsia: assign(255, 0, 255); break;
                case Green: assign(0, 128, 0); break;
                case Lime: assign(0, 255, 0); break;
                case Magenta: assign(255, 0, 255); break;
                case Orange: assign(255, 165, 0); break;
                case Purple: assign(128, 0, 128); break;
                case Red: assign(255, 0, 0); break;
                case Silver: assign(192, 192, 192); break;
                case White: assign(255, 255, 255); break;
                case Yellow: assign(255, 255, 0); break;
                default: transparent = true; break;
            }
        }
        ~Color() override = default;
        std::string toString(Layout const &) const override
        {
            std::stringstream ss;
            if (transparent)
                ss << "none";
            else
                ss << "rgb(" << red << "," << green << "," << blue << ")";
            return ss.str();
        }
    private:
            bool transparent;
            int red;
            int green;
            int blue;

            void assign(int r, int g, int b)
            {
                red = r;
                green = g;
                blue = b;
            }
    };

    class Fill : public Serializable
    {
    public:
        Fill(Color::Defaults color) : color(color) { }
        Fill(const Color & color = Color::Transparent)
            : color(color) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << attribute("fill", color.toString(layout));
            return ss.str();
        }
    private:
        Color color;
    };

    class Stroke : public Serializable
    {
    public:
        explicit Stroke(double width = -1, Color const & color = Color::Transparent, bool nonScalingStroke = false)
            : width(width), color(color), nonScaling(nonScalingStroke) { }
        std::string toString(Layout const & layout) const override
        {
            // If stroke width is invalid.
            if (width < 0)
                return std::string();

            std::stringstream ss;
            ss << attribute("stroke-width", translateScale(width, layout)) << attribute("stroke", color.toString(layout));
            if (nonScaling)
               ss << attribute("vector-effect", "non-scaling-stroke");
            return ss.str();
        }
    private:
        double width;
        Color color;
        bool nonScaling;
    };

    class Font : public Serializable
    {
    public:
        explicit Font(double size = 12, std::string const & family = "Verdana") : size(size), family(family) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << attribute("font-size", translateScale(size, layout)) << attribute("font-family", family);
            return ss.str();
        }
    private:
        double size;
        std::string family;
    };

    class Shape : public Serializable
    {
    public:
        explicit Shape(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : fill(fill), stroke(stroke) { }
        ~Shape() override = default;
        std::string toString(Layout const & layout) const override = 0;
        virtual void offset(Point const & offset) = 0;
        virtual Shape *clone() const = 0;
    protected:
        Fill fill;
        Stroke stroke;
    };
    template <typename T>
    inline std::string vectorToString(std::vector<T> collection, Layout const & layout)
    {
        std::string combination_str;
        for (size_t i = 0; i < collection.size(); ++i)
            combination_str += collection[i].toString(layout);

        return combination_str;
    }

    class Circle : public Shape
    {
    public:
        Circle(Point const & center, double diameter, Fill const & fill,
            Stroke const & stroke = Stroke())
            : Shape(fill, stroke), center(center), radius(diameter / 2) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("circle") << attribute("cx", translateX(center.x, layout))
                << attribute("cy", translateY(center.y, layout))
                << attribute("r", translateScale(radius, layout)) << fill.toString(layout)
                << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            center.x += offset.x;
            center.y += offset.y;
        }
        Circle *clone() const override {
            return new Circle(*this);
        }
    private:
        Point center;
        double radius;
    };

    class Elipse : public Shape
    {
    public:
        Elipse(Point const & center, double width, double height,
            Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), center(center), radius_width(width / 2),
            radius_height(height / 2) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("ellipse") << attribute("cx", translateX(center.x, layout))
                << attribute("cy", translateY(center.y, layout))
                << attribute("rx", translateScale(radius_width, layout))
                << attribute("ry", translateScale(radius_height, layout))
                << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            center.x += offset.x;
            center.y += offset.y;
        }
        Elipse *clone() const override {
            return new Elipse(*this);
        }
    private:
        Point center;
        double radius_width;
        double radius_height;
    };

    class Rectangle : public Shape
    {
    public:
        Rectangle(Point const & edge, double width, double height,
            Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), edge(edge), width(width),
            height(height) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("rect") << attribute("x", translateX(edge.x, layout))
                << attribute("y", translateY(edge.y, layout))
                << attribute("width", translateScale(width, layout))
                << attribute("height", translateScale(height, layout))
                << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            edge.x += offset.x;
            edge.y += offset.y;
        }
        Rectangle *clone() const override {
            return new Rectangle(*this);
        }
    private:
        Point edge;
        double width;
        double height;
    };

    class Line : public Shape
    {
    public:
        Line(Point const & start_point, Point const & end_point,
            Stroke const & stroke = Stroke())
            : Shape(Fill(), stroke), start_point(start_point),
            end_point(end_point) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("line") << attribute("x1", translateX(start_point.x, layout))
                << attribute("y1", translateY(start_point.y, layout))
                << attribute("x2", translateX(end_point.x, layout))
                << attribute("y2", translateY(end_point.y, layout))
                << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            start_point.x += offset.x;
            start_point.y += offset.y;

            end_point.x += offset.x;
            end_point.y += offset.y;
        }
        Line *clone() const override {
            return new Line(*this);
        }
    private:
        Point start_point;
        Point end_point;
    };

    class Polygon : public Shape
    {
    public:
         explicit Polygon(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke) { }
        explicit Polygon(Stroke const & stroke) : Shape(Color::Transparent, stroke) { }
        Polygon & operator<<(Point const & point)
        {
            points.push_back(point);
            return *this;
        }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("polygon");

            ss << "points=\"";
            for (auto & point : points)
                ss << translateX(point.x, layout) << "," << translateY(point.y, layout) << " ";
            ss << "\" ";

            ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            for (auto & point : points) {
                point.x += offset.x;
                point.y += offset.y;
            }
        }
        Polygon *clone() const override {
             return new Polygon(*this);
         }
    private:
        std::vector<Point> points;
    };

    class Path : public Shape
    {
    public:
       explicit Path(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
          : Shape(fill, stroke)
       {  startNewSubPath(); }
       explicit Path(Stroke const & stroke = Stroke()) : Shape(Color::Transparent, stroke)
       {  startNewSubPath(); }
       Path & operator<<(Point const & point)
       {
          paths.back().push_back(point);
          return *this;
       }

       void startNewSubPath()
       {
          if (paths.empty() || !paths.back().empty())
            paths.emplace_back();
       }

       std::string toString(Layout const & layout) const override
       {
          std::stringstream ss;
          ss << elemStart("path");

          ss << "d=\"";
          for (auto const& subpath: paths)
          {
             if (subpath.empty())
                continue;

             ss << "M";
             for (auto const& point: subpath)
                ss << translateX(point.x, layout) << "," << translateY(point.y, layout) << " ";
             ss << "z ";
          }
          ss << "\" ";
          ss << "fill-rule=\"evenodd\" ";

          ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
          return ss.str();
       }
       void offset(Point const & offset) override
       {
          for (auto& subpath : paths)
             for (auto& point : subpath)
             {
                point.x += offset.x;
                point.y += offset.y;
             }
       }
       Path *clone() const override {
           return new Path(*this);
       }
    private:
       std::vector<std::vector<Point>> paths;
    };

    class Polyline : public Shape
    {
    public:
        explicit Polyline(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke) { }
        explicit Polyline(Stroke const & stroke) : Shape(Color::Transparent, stroke) { }
        explicit Polyline(std::vector<Point> const & points,
            Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), points(points) { }
        Polyline & operator<<(Point const & point)
        {
            points.push_back(point);
            return *this;
        }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("polyline");

            ss << "points=\"";
            for (auto & point : points)
                ss << translateX(point.x, layout) << "," << translateY(point.y, layout) << " ";
            ss << "\" ";

            ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            for (auto & point : points) {
                point.x += offset.x;
                point.y += offset.y;
            }
        }
        Polyline *clone() const override {
            return new Polyline(*this);
        }
        std::vector<Point> points;
    };

    class Text : public Shape
    {
    public:
        Text(Point const & origin, std::string const & content, Fill const & fill = Fill(),
             Font const & font = Font(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), origin(origin), content(content), font(font) { }
        std::string toString(Layout const & layout) const override
        {
            std::stringstream ss;
            ss << elemStart("text") << attribute("x", translateX(origin.x, layout))
                << attribute("y", translateY(origin.y, layout))
                << fill.toString(layout) << stroke.toString(layout) << font.toString(layout)
                << ">" << content << elemEnd("text");
            return ss.str();
        }
        void offset(Point const & offset) override
        {
            origin.x += offset.x;
            origin.y += offset.y;
        }
        Text *clone() const override {
            return new Text(*this);
        }
    private:
        Point origin;
        std::string content;
        Font font;
    };

    // Sample charting class.
    class LineChart : public Shape
    {
    public:
        explicit LineChart(Dimensions margin = Dimensions(), double scale = 1,
                           Stroke const & axis_stroke = Stroke(.5, Color::Purple))
                : axis_stroke(axis_stroke), margin(margin), scale(scale) { }
        LineChart & operator<<(Polyline const & polyline)
        {
            if (polyline.points.empty())
                return *this;

            polylines.push_back(polyline);
            return *this;
        }
        std::string toString(Layout const & layout) const override
        {
            if (polylines.empty())
                return "";

            std::string ret;
            for (auto const & polyline : polylines)
                ret += polylineToString(polyline, layout);

            return ret + axisString(layout);
        }
        void offset(Point const & offset) override
        {
            for (auto & polyline : polylines)
                polyline.offset(offset);
        }
        LineChart *clone() const override {
            return new LineChart(*this);
        }
    private:
        Stroke axis_stroke;
        Dimensions margin;
        double scale;
        std::vector<Polyline> polylines;

        optional<Dimensions> getDimensions() const
        {
            if (polylines.empty())
                return optional<Dimensions>();

            optional<Point> min = getMinPoint(polylines[0].points);
            optional<Point> max = getMaxPoint(polylines[0].points);
            for (auto const & polyline : polylines) {
                if (getMinPoint(polyline.points)->x < min->x)
                    min->x = getMinPoint(polyline.points)->x;
                if (getMinPoint(polyline.points)->y < min->y)
                    min->y = getMinPoint(polyline.points)->y;
                if (getMaxPoint(polyline.points)->x > max->x)
                    max->x = getMaxPoint(polyline.points)->x;
                if (getMaxPoint(polyline.points)->y > max->y)
                    max->y = getMaxPoint(polyline.points)->y;
            }

            return optional<Dimensions>(Dimensions(max->x - min->x, max->y - min->y));
        }
        std::string axisString(Layout const & layout) const
        {
            optional<Dimensions> dimensions = getDimensions();
            if (!dimensions)
                return "";

            // Make the axis 10% wider and higher than the data points.
            double width = dimensions->width * 1.1;
            double height = dimensions->height * 1.1;

            // Draw the axis.
            Polyline axis(Color::Transparent, axis_stroke);
            axis << Point(margin.width, margin.height + height) << Point(margin.width, margin.height)
                 << Point(margin.width + width, margin.height);

            return axis.toString(layout);
        }
        std::string polylineToString(Polyline const & polyline, Layout const & layout) const
        {
            Polyline shifted_polyline = polyline;
            shifted_polyline.offset(Point(margin.width, margin.height));

            std::vector<Circle> vertices;
            for (auto const & point : shifted_polyline.points)
                vertices.emplace_back(point, getDimensions()->height / 30.0, Color::Black);

            return shifted_polyline.toString(layout) + vectorToString(vertices, layout);
        }
    };

    class Container : public Shape {
    public:
        Container(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
                : Shape(fill, stroke) { }
        Container(Stroke const & stroke) : Shape(Color::Transparent, stroke) { }
        Container(Container const & copy) : Shape(copy) {
            for (auto & child : copy.childs) {
                childs.emplace_back(child.get()->clone());
            }
        }
        Container & operator<<(Shape const & child)
        {
            childs.emplace_back(child.clone());
            return *this;
        }
        std::string toString(Layout const & layout) const override
        {
            if (childs.empty()) return "";

            std::string strokeStr = stroke.toString(layout);

            std::stringstream ss;
            ss << elemStart("g");
            ss << fill.toString(layout) << stroke.toString(layout) << ">\n";

            for (std::unique_ptr<Shape> const & child : childs) {
                ss << indent(child->toString(layout));
            }

            ss << elemEnd("g");

            return ss.str();
        }
        void offset(Point const & offset) override { }
        Container *clone() const override {
            return new Container(*this);
        }
    private:
        std::vector<std::unique_ptr<Shape>> childs;
    };

    class Document
    {
    public:
        explicit Document(std::string const & file_name, Layout layout = Layout())
            : file_name(file_name), layout(layout) { }

        Document & operator<<(Shape const & shape)
        {
            body_nodes_str_list.push_back(shape.toString(layout));
            return *this;
        }
        std::string toString() const
        {
            std::stringstream ss;
            writeToStream(ss);
            return ss.str();
        }
        bool save() const
        {
            std::ofstream ofs(file_name.c_str());
            if (!ofs.good())
                return false;

            writeToStream(ofs);
            ofs.close();
            return true;
        }
    private:
        void writeToStream(std::ostream& str) const
        {
            str << "<?xml " << attribute("version", "1.0") << attribute("standalone", "no")
                << "?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
                << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg "
                << attribute("width", layout.dimensions.width, "px")
                << attribute("height", layout.dimensions.height, "px")
                << attribute("xmlns", "http://www.w3.org/2000/svg")
                << attribute("version", "1.1") << ">\n";
            for (const auto& body_node_str : body_nodes_str_list) {
                str << indent(body_node_str);
            }
            str << elemEnd("svg");
        }

    private:
        std::string file_name;
        Layout layout;

        std::vector<std::string> body_nodes_str_list;
    };
}
#endif
