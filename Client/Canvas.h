#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include <vector>
#include "Shape.h"
#include "Area.h"
#include "Node.h"
#include "Connection.h"

class MyCanvas : public wxPanel {
public:
    MyCanvas(wxWindow* parent, wxListBox* shapeList);
    ~MyCanvas();

    void AddNewArea();
    void AddNewNode();
    void SelectShape(int index);
    void SaveToFile(const std::string& path);
    void LoadFromFile(const std::string& path);

private:
    std::vector<Shape*> shapes;
    wxListBox* shapeList;

    size_t selectedIndex = -1;
    wxPoint lastMouse;
    double scale = 1.0;
    wxPoint2DDouble offset = { 0, 0 };

    bool dragging = false;
    bool resizing = false;
    bool panning = false;
    HandleType activeHandle = HandleType::None;

    const Port* pendingPort = nullptr;
    std::vector<Connection> connections;

    const Shape* pendingShape = nullptr;

    void RefreshList();

    void OnPaint(wxPaintEvent& evt);
    void OnMouseWheel(wxMouseEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnLeftDClick(wxMouseEvent& evt);

    const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const;

    wxDECLARE_EVENT_TABLE();
};
