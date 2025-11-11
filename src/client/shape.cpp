#include "innerstat/client/shape.h"
#include "innerstat/client/canvas.h"
#include "innerstat/client/PropertyDialog.h"
#include "innerstat/client/client.h" 
#include "innerstat/client/color_manager.h" // Include for color constants
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <sstream>
#include <format>
#include <stdio.h>
#include <string>

INNERSTAT_BEGIN_NAMESPACE

Shape::Shape(int x, int y, int w, int h, 
    MainCanvas* canvas, Shape* parent, const std::string& label,
    const ShapeType& Type, int portCount)
    : rect(wxPoint(x,y), wxSize(w, h)), canvas(canvas), parent(parent), label(label),
        scale(&(this->canvas->scale)), offset(&(this->canvas->offset)), type(Type) {
    SetPortCount(portCount);
    last_seen_timestamp = wxGetUTCTimeMillis();

    if (type == ShapeType::OS) {
        size_t first_paren = label.find('(');
        if (first_paren != std::string::npos) {
            mac_address = label.substr(0, first_paren);
        } else {
            mac_address = label;
        }
    }
}

Shape::~Shape(){
    for(Shape* area : childAreas){
        delete area;
    }
    childAreas.clear();
}

void Shape::Draw(wxGraphicsContext& gc) const {
    wxRect screenRect(GetScreenRect());
    
    // 1. Draw base shape
    wxColour fillColour;
    switch (this->GetType())
    {
        case ShapeType::OS:
            fillColour = isSelected ? C_OS_SELECTED_FILL : C_OS_FILL;
            break;
        case ShapeType::PS:
            fillColour = isSelected ? C_PS_SELECTED_FILL : C_PS_FILL;
            break;
        default:
            fillColour = isSelected ? C_SHAPE_SELECTED_FILL : C_SHAPE_FILL;
            break;
    }
    wxColour borderColour = isSelected ? C_SHAPE_SELECTED_BORDER : C_SHAPE_BORDER;
    gc.SetBrush(wxBrush(fillColour));
    gc.SetPen(wxPen(borderColour, std::max(1, (int)(2 * (*scale)))));
    gc.DrawRoundedRectangle(screenRect.GetX(), screenRect.GetY(), screenRect.GetWidth(), screenRect.GetHeight(), (int)(10 * (*scale)));

    // 2. Draw status hatching on top
    ShapeStatus current_status = this->status;
    if (current_status != ShapeStatus::Normal) {
        wxColour hatchColour;
        if (current_status == ShapeStatus::Attention) hatchColour.Set(255, 165, 0, 192); // Orange, thicker alpha
        else if (current_status == ShapeStatus::Warning) hatchColour.Set(255, 0, 0, 192);   // Red, thicker alpha
        else if (current_status == ShapeStatus::Lost) hatchColour.Set(0, 0, 0, 192);      // Black, thicker alpha

        gc.SetBrush(wxBrush(hatchColour, wxBRUSHSTYLE_CROSSDIAG_HATCH));
        gc.SetPen(*wxTRANSPARENT_PEN);
        gc.DrawRoundedRectangle(screenRect.GetX(), screenRect.GetY(), screenRect.GetWidth(), screenRect.GetHeight(), (int)(10 * (*scale)));
    }
    
    // 3. Draw Text
    std::string display_label = this->label;
    if (this->GetType() == ShapeType::PS) {
        display_label += ":" + std::to_string(this->port_number);
    }

    std::string debug_label = display_label;
    if (canvas->isdebug) {
        if (current_status == ShapeStatus::Normal) debug_label = "[N] " + debug_label;
        else if (current_status == ShapeStatus::Attention) debug_label = "[A] " + debug_label;
        else if (current_status == ShapeStatus::Warning) debug_label = "[W] " + debug_label;
        else if (current_status == ShapeStatus::Lost) debug_label = "[L] " + debug_label;
    }
    
    wxFont font((int)(9 * (*scale)), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    gc.SetFont(font, C_SHAPE_LABEL);
    
    // Use GetTextExtent to calculate position for centering or other alignment if needed
    double text_width, text_height;
    gc.GetTextExtent(debug_label, &text_width, &text_height);
    gc.DrawText(debug_label, screenRect.GetPosition().x + 5, screenRect.GetPosition().y + screenRect.GetHeight() + 5);

    // 4. Draw Ports
    for (const Port& port : ports) {
        port.Draw(gc, screenRect);
    }
    
    // 5. Draw selection handle
    if(isSelected){
        gc.SetPen(wxPen(*wxBLACK, 1));
        gc.SetBrush(*wxBLACK_BRUSH);
        const int handleScale = 6;
        gc.DrawRectangle(screenRect.GetBottomRight().x - handleScale / 2, screenRect.GetBottomRight().y - handleScale / 2, handleScale, handleScale);
    }
}

void Shape::AddChildArea(Shape* area){
    childAreas.push_back(area);
}

void Shape::OpenPropertyDialog(){
    PropertyDialog dialog(canvas, this);
    if (dialog.ShowModal() == wxID_OK) {
        canvas->Refresh();
    }
}

void Shape::OpenAddShapeDialog() {
    // This function is deprecated and its logic moved to AgentSelectionDialog.
    // It can be removed or left empty.
}

void Shape::SetPortCount(int count) {
    ports.clear();
    for (int i = 0; i < count; ++i) {
        double ratio = (i + 1.0) / (count + 1.0);
        std::string id = label + "_p" + std::to_string(i);
        ports.emplace_back(canvas, id, wxPoint2DDouble(ratio, 0.0));
    }
}

const Port* Shape::HitTestPort(const wxPoint& pos, const Shape** outShape) const {
    const std::vector<Port>& ports = this->GetPorts();
    for (const Port& port : ports) {
        wxPoint screenPos = port.GetScreenPosition(GetScreenRect());
        wxRect hitbox(screenPos.x - 6, screenPos.y - 6, 12, 12);
        if (hitbox.Contains(pos)) {
            if (outShape) *outShape = this;
            return &port;
        }
    }
    return nullptr;
}

ShapeHandle Shape::HitTestShape(wxPoint& mouse) {
    const int hs = 6; // 핸들 크기 (6x6)

    wxRect scaledRect(GetScreenRect());

    wxPoint handles[8] = {
        scaledRect.GetTopLeft(),                                                            // TopLeft
        wxPoint(scaledRect.GetLeft() + scaledRect.GetWidth() / 2, scaledRect.GetTop()),     // Top
        scaledRect.GetTopRight(),                                                           // TopRight
        wxPoint(scaledRect.GetLeft(), scaledRect.GetTop() + scaledRect.GetHeight() / 2),    // Left
        wxPoint(scaledRect.GetRight(), scaledRect.GetTop() + scaledRect.GetHeight() / 2),   // Right
        scaledRect.GetBottomLeft(),                                                         // BottomLeft
        wxPoint(scaledRect.GetLeft() + scaledRect.GetWidth() / 2, scaledRect.GetBottom()),  // Bottom
        scaledRect.GetBottomRight()                                                         // BottomRight
    };

    for (int i = 0; i < 8; ++i) {
        wxRect handle(handles[i].x - hs / 2, handles[i].y - hs / 2, hs, hs);
        if (handle.Contains(mouse))
            return ShapeHandle(this, static_cast<HandleType>(i));
    }

    if (scaledRect.Contains(mouse))
        return ShapeHandle(this, HandleType::Body);

    return ShapeHandle(nullptr, HandleType::None);
}


std::string Shape::Serialize() const {
    // TODO
    return "";
}

Shape* Shape::Deserialize(const std::string& line, MainCanvas* canvas) {
    // TODO
    return nullptr;
}

INNERSTAT_END_NAMESPACE
