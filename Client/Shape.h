#pragma once
#include <wx/wx.h>
#include "Port.h"

enum class HandleType {
    None = -1,
    TopLeft, Top, TopRight,
    Left, Right,
    BottomLeft, Bottom, BottomRight
};

class Shape {
public:
    wxPoint2DDouble pos;
    double width, height;

    Shape(double x, double y, double w, double h);
    virtual ~Shape() = default;

    virtual void Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset, bool selected) const = 0;
    virtual void OpenPropertyDialog(wxWindow* parent) { }
    virtual bool Contains(const wxPoint& screenPt, double scale, const wxPoint2DDouble& offset) const;
    virtual std::string Serialize() const = 0;
    HandleType HitTestHandle(const wxPoint& screenPt, double scale, const wxPoint2DDouble& offset) const;

    virtual const std::vector<Port>& GetPorts() const {
        static std::vector<Port> empty;
        return empty;
    }
};
