#include "innerstat/client/AddShapeDialog.h"

AddShapeDialog::AddShapeDialog(wxWindow* parent, const systemInfo& agent_info)
    : wxDialog(parent, wxID_ANY, "Add Shape", wxDefaultPosition, wxSize(600, 400)),
      all_processes_(agent_info.lsof_items) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Label
    wxFlexGridSizer* fgSizer = new wxFlexGridSizer(1, 2, 5, 5);
    fgSizer->Add(new wxStaticText(this, wxID_ANY, "Label:"), 0, wxALIGN_CENTER_VERTICAL);
    wxString default_label = wxString::Format("%s(%s)", agent_info.status.mac_address, agent_info.status.os);
    label_ctrl_ = new wxTextCtrl(this, wxID_ANY, default_label);
    fgSizer->Add(label_ctrl_, 1, wxEXPAND);
    fgSizer->AddGrowableCol(1);
    mainSizer->Add(fgSizer, 0, wxEXPAND | wxALL, 10);

    // Process List
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Select processes to add as child shapes:"), 0, wxLEFT | wxTOP, 10);
    wxArrayString process_choices;
    for (const auto& proc : all_processes_) {
        // Format: "COMMAND (PID) - NAME"
        wxString choice = wxString::Format("%s (%s) - %s", proc.cmd, proc.pid, proc.name);
        process_choices.Add(choice);
    }
    process_list_box_ = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, process_choices);
    mainSizer->Add(process_list_box_, 1, wxEXPAND | wxALL, 10);

    wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    SetSizerAndFit(mainSizer);
    Centre();
}

std::string AddShapeDialog::GetShapeLabel() const {
    return label_ctrl_->GetValue().ToStdString();
}

std::vector<LsofItem> AddShapeDialog::GetSelectedProcesses() const {
    std::vector<LsofItem> selected;
    for (unsigned int i = 0; i < process_list_box_->GetCount(); ++i) {
        if (process_list_box_->IsChecked(i)) {
            selected.push_back(all_processes_[i]);
        }
    }
    return selected;
}
