#include "innerstat/client/agent_mqtt_connection.h"
#include "innerstat/client/AgentSelectionDialog.h" // Include for casting
#include <wx/wx.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <mosquitto.h>

AgentMqttConnection* AgentMqttConnection::instance = nullptr;

AgentMqttConnection* AgentMqttConnection::GetInstance() {
    if (instance == nullptr) {
        instance = new AgentMqttConnection();
    }
    return instance;
}

AgentMqttConnection::AgentMqttConnection() : mosquittopp("innerstat_client") {
    mosqpp::lib_init();
    connect("localhost", 1883, 60);
    loop_start();
}

AgentMqttConnection::~AgentMqttConnection() {
    loop_stop(true);
    mosqpp::lib_cleanup();
}

void AgentMqttConnection::on_connect(int rc) {
    if (rc == 0) {
        subscribe(NULL, "innerstat/+/info");
        subscribe(NULL, "innerstat/+/ps");
        subscribe(NULL, "innerstat/+/+/log");
        // Actively request info from all agents upon connection
        publish(NULL, "innerstat/broadcast/request_info", 1, "1", 0, false);
    } else {
        wxLogError("Client connection failed.");
    }
}

void AgentMqttConnection::on_message(const struct mosquitto_message* message) {
    std::string topic = message->topic;
    std::string payload = std::string((char*)message->payload, message->payloadlen);

    bool is_info_topic = false;
    mosquitto_topic_matches_sub("innerstat/+/info", topic.c_str(), &is_info_topic);

    bool is_ps_topic = false;
    mosquitto_topic_matches_sub("innerstat/+/ps", topic.c_str(), &is_ps_topic);

    bool is_log_topic = false;
    mosquitto_topic_matches_sub("innerstat/+/+/log", topic.c_str(), &is_log_topic);

    std::string mac_address;
    std::stringstream ss(topic);
    std::string segment;
    std::vector<std::string> seglist;
    while(std::getline(ss, segment, '/'))
    {
       seglist.push_back(segment);
    }

    if (seglist.size() > 1) {
        mac_address = seglist[1];
        last_agent_mac_ = mac_address;
    }

    if (is_info_topic) {
        bool is_new = (std::find(known_agents_.begin(), known_agents_.end(), mac_address) == known_agents_.end());
        
        systemInfo info(payload);
        agent_data_.insert_or_assign(mac_address, std::make_pair(info, wxGetUTCTimeMillis()));

        if (is_new) {
            known_agents_.push_back(mac_address);
            if (agent_change_handler_) {
                // Cast is necessary because CallAfter is a template method
                auto* dialog = static_cast<innerstat::AgentSelectionDialog*>(agent_change_handler_);
                dialog->CallAfter(&innerstat::AgentSelectionDialog::UpdateAgentList);
            }
        }

        if (lsofText && eventHandler) {
            std::string display_payload = "[" + mac_address + "]\n" + payload;
            eventHandler->CallAfter([this, display_payload]() {
                lsofText->SetValue(display_payload);
            });
        }
    } else if (is_ps_topic) {
        if (psText && eventHandler) {
            std::string display_payload = "[" + mac_address + "]\n" + payload;
            eventHandler->CallAfter([this, display_payload]() {
                psText->SetValue(display_payload);
            });
        }
    }
    else if (is_log_topic) {
        if (seglist.size() > 2) {
            try {
                int port = std::stoi(seglist[2]);
                all_log_timestamps_[mac_address][port].push_back(wxGetUTCTimeMillis());
            } catch (...) {
                // ignore if port is not a valid number
            }
        }
    }
}

void AgentMqttConnection::SendCommand(const std::string& mac, const std::string& command) {
    std::string command_topic = "innerstat/" + mac + "/command";
    publish(NULL, command_topic.c_str(), command.length(), command.c_str());
}

systemInfo AgentMqttConnection::GetAgentData(const std::string& mac) {
    auto it = agent_data_.find(mac);
    if (it != agent_data_.end()) {
        return it->second.first;
    }
    // Return a default-constructed systemInfo if not found
    std::string empty_data = "{}";
    return systemInfo(empty_data);
}