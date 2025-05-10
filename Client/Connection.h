#pragma once
#include "Port.h"
#include "Shape.h"

class Connection
{
public:
    const Port *from;
    const Port *to;
    const Shape *fromShape;
    const Shape *toShape;

    Connection(const Port* f, const Port* t)
        : from(f), to(t), fromShape(nullptr), toShape(nullptr) {}

    Connection(const Port* f, const Port* t, const Shape* fs, const Shape* ts)
        : from(f), to(t), fromShape(fs), toShape(ts) {}
        
    void Draw(wxDC &dc, double scale, const wxPoint2DDouble &offset) const;
};