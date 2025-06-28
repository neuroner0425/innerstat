#include "innerstat/client/dialog.h"
#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif
#include "innerstat/client/shape.h"
#include "innerstat/client/area.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

INNERSTAT_BEGIN_NAMESPACE

AreaProperties* ShowAddAreaDialog(wxWindow* parent){
    wxDialog dlg(parent, wxID_ANY, "Add Area");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    wxBoxSizer* midSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxArrayString types = { "Other", "OS", "VM", "Container", "Network" };
    
    wxTextCtrl* labelCtrl = new wxTextCtrl(&dlg, wxID_ANY, wxString("New Area"));
    wxChoice* typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);

    typeCtrl->SetSelection(1);

    midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
    midSizer->Add(labelCtrl, 0, wxEXPAND | wxALL, 5);
    midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
    midSizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);

    bottomSizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALL, 10);
    bottomSizer->Add(new wxButton(&dlg, wxID_CANCEL), 0, wxALL, 10);
    
    sizer->Add(midSizer, 1, wxEXPAND | wxALL, 10);
    sizer->Add(bottomSizer, 0, wxALIGN_CENTER);
    
    dlg.SetSizerAndFit(sizer);
    dlg.SetSize(wxSize(200, -1));

    int status = dlg.ShowModal();
    if (status == wxID_OK) {
        return new AreaProperties(labelCtrl->GetValue().ToStdString(), static_cast<AreaType>(typeCtrl->GetSelection()), 1);
    }
    return nullptr;
}

ShapeProperties* ShowAddShapeDialog(wxWindow* parent) {
    wxDialog dlg(parent, wxID_ANY, "Add Shape");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* midSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString shapeTypes = { "Area", "Node" };
    wxArrayString types = { "Other", "OS", "VM", "Container", "Network" };

    wxChoice* shapeTypeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, shapeTypes);
    shapeTypeCtrl->SetSelection(0);

    wxTextCtrl* labelCtrl = nullptr;
    wxChoice* typeCtrl = nullptr;

    topSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Shape Type:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    topSizer->Add(shapeTypeCtrl, 0, wxALL, 10);

    bottomSizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALL, 10);
    bottomSizer->Add(new wxButton(&dlg, wxID_CANCEL), 0, wxALL, 10);

    sizer->Add(topSizer, 0, wxEXPAND);
    sizer->Add(midSizer, 1, wxEXPAND | wxALL, 10);
    sizer->Add(bottomSizer, 0, wxALIGN_CENTER);

    dlg.SetSizerAndFit(sizer);
    dlg.SetSize(wxSize(200, -1));

    auto updateMidSizer = [&]() {
        midSizer->Clear(true);

        midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
        labelCtrl = new wxTextCtrl(&dlg, wxID_ANY, wxString("New Node"));
        
        midSizer->Add(labelCtrl, 0, wxEXPAND | wxALL, 5);

        if (shapeTypeCtrl->GetSelection() == 0) {
            labelCtrl->SetValue(wxString("New Area"));
            midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
            typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
            typeCtrl->SetSelection(0);
            midSizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);
        } else {
            typeCtrl = nullptr;
        }

        dlg.Layout();
        dlg.Fit();
    };

    updateMidSizer();

    shapeTypeCtrl->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) {
        updateMidSizer();
    });

    int status = dlg.ShowModal();
    if (status == wxID_OK) {
        std::string label = labelCtrl->GetValue().ToStdString();
        if (shapeTypeCtrl->GetSelection() == 0) {
            AreaType areaType = static_cast<AreaType>(typeCtrl->GetSelection());
            return new AreaProperties(label, areaType, 0);
        } else {
            return new NodeProperties(label, 0);
        }
    }
    return nullptr;
}

AreaProperties* ShowAreaPropertyDialog(wxWindow* parent, Area* area) {
    wxDialog dlg(parent, wxID_ANY, "Area Properties");
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString types = { "Other", "OS", "VM", "Container", "Network" };

    wxTextCtrl *labelctrl = new wxTextCtrl(&dlg, wxID_ANY, wxString(area->label), wxDefaultPosition, wxDefaultSize);
    wxChoice *typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
    wxSpinCtrl *portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    wxButton *addShapeBtn = new wxButton(&dlg, wxID_ANY, "Add Shape");

    typeCtrl->SetSelection(static_cast<int>(area->type));
    portCount->SetRange(0, 10);
    portCount->SetValue(area->ports.size());

    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
    sizer->Add(labelctrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
    sizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Port Count:"), 0, wxALL, 5);
    sizer->Add(portCount, 0, wxEXPAND | wxALL, 5);
    sizer->Add(addShapeBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizerAndFit(sizer);
    dlg.SetSize(wxSize(200, -1));

    addShapeBtn->Bind(wxEVT_BUTTON,[=](wxCommandEvent &evt){
        area->OpenAddShapeDialog();
    });

    if (dlg.ShowModal() == wxID_OK) {
        return new AreaProperties(labelctrl->GetValue().ToStdString(), static_cast<AreaType>(typeCtrl->GetSelection()), portCount->GetValue());
    }
    return nullptr;
}

INNERSTAT_END_NAMESPACE