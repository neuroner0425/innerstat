#include "innerstat/client/dialog.h"
#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif
#include "innerstat/client/shape.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

INNERSTAT_BEGIN_NAMESPACE

AreaProperties* ShowAddAreaDialog(wxWindow* parent, int defaultType){
    wxDialog dlg(parent, wxID_ANY, "Add Shape");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    wxBoxSizer* midSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxArrayString types = { "Other", "PS", "OS", "VM", "Container", "Network" };
    
    wxTextCtrl* labelCtrl = new wxTextCtrl(&dlg, wxID_ANY, wxString("New " + types[defaultType]));
    wxChoice* typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);

    int beforeSelection = defaultType;

    typeCtrl->SetSelection(defaultType);

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

    typeCtrl->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
        if (labelCtrl->GetValue() == "New " + types[beforeSelection])
            labelCtrl->SetValue("New " + types[typeCtrl->GetSelection()]);
        beforeSelection = typeCtrl->GetSelection();
    });

    int status = dlg.ShowModal();
    if (status == wxID_OK) {
        return new AreaProperties(labelCtrl->GetValue().ToStdString(), static_cast<ShapeType>(typeCtrl->GetSelection()), 1);
    }
    return nullptr;
}

AreaProperties* ShowAreaPropertyDialog(wxWindow* parent, Shape* area) {
    wxDialog dlg(parent, wxID_ANY, "Properties");
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString types = { "Other", "PS", "OS", "VM", "Container", "Network" };

    wxTextCtrl *labelctrl = new wxTextCtrl(&dlg, wxID_ANY, wxString(area->label), wxDefaultPosition, wxDefaultSize);
    wxChoice *typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
    wxSpinCtrl *portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    wxButton *addShapeBtn = new wxButton(&dlg, wxID_ANY, "Add Shape");

    typeCtrl->SetSelection(static_cast<int>(area->GetType()));
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
        return new AreaProperties(labelctrl->GetValue().ToStdString(), static_cast<ShapeType>(typeCtrl->GetSelection()), portCount->GetValue());
    }
    return nullptr;
}

INNERSTAT_END_NAMESPACE