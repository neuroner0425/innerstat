#include "innerstat/client/PropertyDialog.h"
#include "innerstat/client/shape.h" // Include full definition for implementation
#include "innerstat/client/client.h" // For ShapeType enum
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

PropertyDialog::PropertyDialog(wxWindow* parent, innerstat::v1::Shape* shape)
    : wxDialog(parent, wxID_ANY, "Properties"), shape_(shape) {

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(2, 10, 10);

    // Common: Label
    grid_sizer->Add(new wxStaticText(this, wxID_ANY, "Label:"), 0, wxALIGN_CENTER_VERTICAL);
    label_ctrl_ = new wxTextCtrl(this, wxID_ANY, shape_->label);
    grid_sizer->Add(label_ctrl_, 1, wxEXPAND);

    if (shape_->GetType() == innerstat::v1::ShapeType::OS) {
        this->SetTitle("OS Properties");
        // MAC Address
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "MAC Address:"), 0, wxALIGN_CENTER_VERTICAL);
        mac_address_ctrl_ = new wxTextCtrl(this, wxID_ANY, shape_->mac_address);
        grid_sizer->Add(mac_address_ctrl_, 1, wxEXPAND);

        // CPU Attention Threshold
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "CPU Attention Threshold (%):"), 0, wxALIGN_CENTER_VERTICAL);
        cpu_attention_ctrl_ = new wxSpinCtrlDouble(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, shape_->attention_threshold_cpu, 1.0);
        grid_sizer->Add(cpu_attention_ctrl_, 1, wxEXPAND);

        // CPU Warning Threshold
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "CPU Warning Threshold (%):"), 0, wxALIGN_CENTER_VERTICAL);
        cpu_warning_ctrl_ = new wxSpinCtrlDouble(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, shape_->warning_threshold_cpu, 1.0);
        grid_sizer->Add(cpu_warning_ctrl_, 1, wxEXPAND);

        // Loss Timeout
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "Loss Timeout (s):"), 0, wxALIGN_CENTER_VERTICAL);
        os_loss_timeout_ctrl_ = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3600, shape_->loss_threshold_seconds);
        grid_sizer->Add(os_loss_timeout_ctrl_, 1, wxEXPAND);

    } else if (shape_->GetType() == innerstat::v1::ShapeType::PS) {
        this->SetTitle("PS Properties");
        // Port Number
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "Port Number:"), 0, wxALIGN_CENTER_VERTICAL);
        port_number_ctrl_ = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, shape_->port_number);
        grid_sizer->Add(port_number_ctrl_, 1, wxEXPAND);

        // Log Attention Threshold
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "Log Attention Threshold (/min):"), 0, wxALIGN_CENTER_VERTICAL);
        log_attention_ctrl_ = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, shape_->attention_threshold_log);
        grid_sizer->Add(log_attention_ctrl_, 1, wxEXPAND);

        // Log Warning Threshold
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "Log Warning Threshold (/min):"), 0, wxALIGN_CENTER_VERTICAL);
        log_warning_ctrl_ = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, shape_->warning_threshold_log);
        grid_sizer->Add(log_warning_ctrl_, 1, wxEXPAND);

        // Loss Timeout
        grid_sizer->Add(new wxStaticText(this, wxID_ANY, "Loss Timeout (s):"), 0, wxALIGN_CENTER_VERTICAL);
        ps_loss_timeout_ctrl_ = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3600, shape_->loss_threshold_seconds_ps);
        grid_sizer->Add(ps_loss_timeout_ctrl_, 1, wxEXPAND);
    }

    main_sizer->Add(grid_sizer, 1, wxEXPAND | wxALL, 10);

    wxStdDialogButtonSizer* button_sizer = new wxStdDialogButtonSizer();
    button_sizer->AddButton(new wxButton(this, wxID_OK));
    button_sizer->AddButton(new wxButton(this, wxID_CANCEL));
    button_sizer->Realize();
    main_sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxALL, 10);

    SetSizerAndFit(main_sizer);
    Centre();

    Bind(wxEVT_BUTTON, &PropertyDialog::OnOk, this, wxID_OK);
}

void PropertyDialog::OnOk(wxCommandEvent& event) {
    shape_->label = label_ctrl_->GetValue().ToStdString();

    if (shape_->GetType() == innerstat::v1::ShapeType::OS) {
        shape_->mac_address = mac_address_ctrl_->GetValue().ToStdString();
        shape_->attention_threshold_cpu = cpu_attention_ctrl_->GetValue();
        shape_->warning_threshold_cpu = cpu_warning_ctrl_->GetValue();
        shape_->loss_threshold_seconds = os_loss_timeout_ctrl_->GetValue();
    } else if (shape_->GetType() == innerstat::v1::ShapeType::PS) {
        shape_->port_number = port_number_ctrl_->GetValue();
        shape_->attention_threshold_log = log_attention_ctrl_->GetValue();
        shape_->warning_threshold_log = log_warning_ctrl_->GetValue();
        shape_->loss_threshold_seconds_ps = ps_loss_timeout_ctrl_->GetValue();
    }

    EndModal(wxID_OK);
}