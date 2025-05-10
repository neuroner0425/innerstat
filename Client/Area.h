#pragma once
#include "Shape.h"
#include "Port.h"
#include <string>
#include <vector>

class Area : public Shape {
public:
    std::string type; // "Hardware", "OS", "VM"
    std::vector<Port> ports;

    Area(double x, double y, double w, double h, const std::string& type);
    void Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset, bool selected) const override;

    void OpenPropertyDialog(wxWindow* parent) override;

    void SetPortCount(int count);

    const std::vector<Port>& GetPorts() const override { return ports; }

    std::string Serialize() const;
    static Area* Deserialize(const std::string& line);
};
