#include "innerstat/client/AddShapeDialog.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/stdpaths.h>

AddShapeDialog::AddShapeDialog(wxWindow* parent, const systemInfo& agent_info)
    : wxDialog(parent, wxID_ANY, "Add Shape", wxDefaultPosition, wxSize(600, 450))
{
    std::string label = agent_info.status.mac_address + "(" + agent_info.status.os + ")";
    label_ctrl_ = new wxTextCtrl(this, wxID_ANY, label);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Label:"), 0, wxALL, 5);
    sizer->Add(label_ctrl_, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Child Processes (Ports):"), 0, wxALL, 5);
    
    wxArrayString process_choices;
    for (const auto& proc : agent_info.lsof_items) {
        if (proc.name.find(':') != std::string::npos) {
            displayable_processes_.push_back(proc);
            process_choices.Add(wxString::Format("%s (%s)", proc.cmd, proc.pid));
        }
    }

    process_list_box_ = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, process_choices);
    sizer->Add(process_list_box_, 1, wxEXPAND | wxALL, 5);

    wxStdDialogButtonSizer* button_sizer = new wxStdDialogButtonSizer();
    button_sizer->AddButton(new wxButton(this, wxID_OK));
    button_sizer->AddButton(new wxButton(this, wxID_CANCEL));
    button_sizer->Realize();

    sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizerAndFit(sizer);
    SetSize(600, 450);
}

std::string AddShapeDialog::GetShapeLabel() const {
    return label_ctrl_->GetValue().ToStdString();
}

std::vector<LsofItem> AddShapeDialog::GetSelectedProcesses() const {
    std::vector<LsofItem> selected;
    for (unsigned int i = 0; i < process_list_box_->GetCount(); ++i) {
        if (process_list_box_->IsChecked(i)) {
            selected.push_back(displayable_processes_[i]);
        }
    }
    return selected;
}
