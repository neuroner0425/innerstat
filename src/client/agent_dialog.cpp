
#include "innerstat/client/agent_dialog.h"
#include "innerstat/client/agent_mqtt_connection.h"

AgentComDialog::AgentComDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Agent Communication", wxDefaultPosition, wxSize(1200, 800), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    psButton = new wxButton(this, wxID_ANY, "ps");
    mainSizer->Add(psButton, 0, wxALL, 5);

    wxBoxSizer* textSizer = new wxBoxSizer(wxHORIZONTAL);
    lsofText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    psText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    textSizer->Add(lsofText, 1, wxEXPAND | wxALL, 5);
    textSizer->Add(psText, 1, wxEXPAND | wxALL, 5);

    mainSizer->Add(textSizer, 1, wxEXPAND);
    SetSizerAndFit(mainSizer);

    psButton->Bind(wxEVT_BUTTON, &AgentComDialog::OnPsButtonClick, this);

    AgentMqttConnection::GetInstance()->SetPsTextCtrl(psText);
    AgentMqttConnection::GetInstance()->SetLsofTextCtrl(lsofText);
    AgentMqttConnection::GetInstance()->SetEventHandler(this);
}

void AgentComDialog::OnPsButtonClick(wxCommandEvent& event) {
    // Request ps from agent
    AgentMqttConnection::GetInstance()->SendCommand("ps");
}