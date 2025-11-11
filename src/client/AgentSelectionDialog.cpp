#include "innerstat/client/AgentSelectionDialog.h"
#include "innerstat/client/agent_mqtt_connection.h"
#include "innerstat/client/AddShapeDialog.h"
#include "innerstat/client/canvas.h"
#include "innerstat/client/shape.h"

INNERSTAT_BEGIN_NAMESPACE

AgentSelectionDialog::AgentSelectionDialog(wxWindow* parent, MainCanvas* canvas)
    : wxDialog(parent, wxID_ANY, "Select Agent", wxDefaultPosition, wxSize(400, 400)),
      canvas_(canvas) {
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    agent_list_box_ = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
    mainSizer->Add(agent_list_box_, 1, wxEXPAND | wxALL, 10);

    wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    SetSizerAndFit(mainSizer);
    SetSize(400, 400);
    Centre();

    // Bind events
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AgentSelectionDialog::OnOk, this, wxID_OK);
    Bind(wxEVT_CLOSE_WINDOW, &AgentSelectionDialog::OnClose, this);

    // Register for updates
    AgentMqttConnection::GetInstance()->SetAgentChangeHandler(this);

    // Initial population
    UpdateAgentList();
}

AgentSelectionDialog::~AgentSelectionDialog() {
    // Ensure handler is cleared
    AgentMqttConnection::GetInstance()->SetAgentChangeHandler(nullptr);
}

void AgentSelectionDialog::OnClose(wxCloseEvent& event) {
    // Clear the handler in the connection manager to avoid dangling pointers
    AgentMqttConnection::GetInstance()->SetAgentChangeHandler(nullptr);
    Destroy(); // Use Destroy() for non-modal dialogs
}

void AgentSelectionDialog::UpdateAgentList() {
    agent_list_box_->Clear();
    mac_addresses_.clear();

    auto all_agent_data = AgentMqttConnection::GetInstance()->GetAgentDataMap();
    for (const auto& pair : all_agent_data) {
        const systemInfo& info = pair.second.first;
        wxString display_string = wxString::Format("%s(%s)", info.status.mac_address, info.status.os);
        agent_list_box_->Append(display_string);
        mac_addresses_.push_back(pair.first);
    }
}

void AgentSelectionDialog::OnOk(wxCommandEvent& event) {
    int selection = agent_list_box_->GetSelection();
    if (selection == wxNOT_FOUND) {
        return;
    }
    
    std::string selected_mac = mac_addresses_[selection];
    systemInfo agent_info = AgentMqttConnection::GetInstance()->GetAgentData(selected_mac);

    AddShapeDialog shape_dialog(this, agent_info);
    if (shape_dialog.ShowModal() != wxID_OK) {
        return;
    }

    std::string label = shape_dialog.GetShapeLabel();
    std::vector<LsofItem> selected_processes = shape_dialog.GetSelectedProcesses();

    int w = (selected_processes.size() > 1) ? 240 : 120;
    int h = (selected_processes.size() + 1) / 2 * 80;
    Shape* parent_shape = canvas_->AddNewArea(w, h, label, ShapeType::OS);

    int child_offset_y = 0;
    bool is_right = false;
    for (const auto& proc : selected_processes) {
        std::string child_label = proc.cmd;
        Shape* child_shape = new Shape(is_right ? 120 : 0, child_offset_y, 80, 40, canvas_, parent_shape, child_label, ShapeType::PS, 0);

        child_shape->port_number = 0; // Default to 0

        // Robust port parsing
        std::string name_field = proc.name;
        size_t arrow_pos = name_field.find("->");
        if (arrow_pos != std::string::npos) {
            name_field = name_field.substr(0, arrow_pos);
        }

        size_t last_colon = name_field.rfind(':');
        if (last_colon != std::string::npos) {
            std::string port_str = name_field.substr(last_colon + 1);
            
            // Trim trailing non-digit characters
            auto it = std::find_if(port_str.rbegin(), port_str.rend(), ::isdigit);
            if (it != port_str.rend()) {
                port_str.erase(it.base(), port_str.end());
            }

            try {
                if (!port_str.empty()) {
                    child_shape->port_number = std::stoi(port_str);
                }
            } catch (...) {
                // port_number remains 0
            }
        }
        
        parent_shape->AddChildArea(child_shape);
        child_offset_y += is_right ? 80 : 0;
        is_right = !is_right;
    }

    canvas_->UpdateAllShapesList();
    canvas_->RefreshTree();
    canvas_->Refresh();

    // We can choose to close the dialog after adding a shape, or leave it open.
    // Let's close it for now.
    Close();
}

INNERSTAT_END_NAMESPACE
