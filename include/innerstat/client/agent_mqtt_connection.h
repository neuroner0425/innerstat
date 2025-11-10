#ifndef INNERSTAT_CLIENT_AGENT_MQTT_CONNECTION_H
#define INNERSTAT_CLIENT_AGENT_MQTT_CONNECTION_H

#include <string>
#include <vector>
#include <map>
#include <mosquittopp.h>
#include <wx/textctrl.h>
#include "innerstat/common/system_info.h"

#include <wx/event.h>

class AgentMqttConnection : public mosqpp::mosquittopp {
public:
    static AgentMqttConnection* GetInstance();
    void SendCommand(const std::string& mac, const std::string& command);
    std::string GetLastAgentMac() const { return last_agent_mac_; }
    std::vector<std::string> GetKnownAgents() const { return known_agents_; }
    systemInfo GetAgentData(const std::string& mac);
    const std::map<std::string, systemInfo>& GetAgentDataMap() const { return agent_data_; }

    void SetAgentChangeHandler(wxEvtHandler* handler) { agent_change_handler_ = handler; }


    void SetPsTextCtrl(wxTextCtrl* ctrl) { psText = ctrl; }
    void SetLsofTextCtrl(wxTextCtrl* ctrl) { lsofText = ctrl; }
    void SetEventHandler(wxEvtHandler* handler) { eventHandler = handler; }

private:
    AgentMqttConnection();
    ~AgentMqttConnection();

    void on_connect(int rc) override;
    void on_message(const struct mosquitto_message* message) override;

    static AgentMqttConnection* instance;
    wxTextCtrl* psText;
    wxTextCtrl* lsofText;
    wxEvtHandler* eventHandler;
    wxEvtHandler* agent_change_handler_ = nullptr;
    std::vector<std::string> known_agents_;
    std::string last_agent_mac_;
    std::map<std::string, systemInfo> agent_data_;
};

#endif // INNERSTAT_CLIENT_AGENT_MQTT_CONNECTION_H