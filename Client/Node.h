#pragma once
#include "Shape.h"
#include "Port.h"
#include <vector>
#include <string>

class Node : public Shape
{
public:
    std::vector<Port> ports;
    std::string pidIdentifier;
    bool active;
    bool overloaded;

    Node(double x, double y, double w, double h, const std::string &pid);
    void Draw(wxDC &dc, double scale, const wxPoint2DDouble &offset, bool selected) const override;

    void OpenPropertyDialog(wxWindow* parent) override;

    void SetPortCount(int count);

    const std::vector<Port>& GetPorts() const override { return ports; }

    std::string Serialize() const;
    static Node* Deserialize(const std::string& line);

};
