#ifndef INNERSTAT_CLIENT_DIALOG_H
#define INNERSTAT_CLIENT_DIALOG_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif
#include <wx/wx.h>

INNERSTAT_BEGIN_NAMESPACE

class Shape;

class ShapeProperties {
public:
    ShapeProperties(std::string label, int portCount)
        : label(label), portCount(portCount){}
    virtual ~ShapeProperties() = default;
    std::string label;
    int portCount;
};

class AreaProperties: public ShapeProperties {
public:
    AreaProperties(std::string label, ShapeType areaType, int portCount)
        : ShapeProperties(label, portCount), areaType(areaType){};
    ShapeType areaType;
};

class NodeProperties: public ShapeProperties {
public:
    NodeProperties(std::string label, int portCount)
        : ShapeProperties(label, portCount){};
};

AreaProperties* ShowAddAreaDialog(wxWindow* parent, int defaultType);
AreaProperties* ShowAreaPropertyDialog(wxWindow* parent, Shape* area);
INNERSTAT_END_NAMESPACE
#endif