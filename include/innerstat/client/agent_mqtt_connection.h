#ifndef INNERSTAT_CLIENT_AGENT_MQTT_CONNECTION_H
#define INNERSTAT_CLIENT_AGENT_MQTT_CONNECTION_H

#include <string>
#include <mosquittopp.h>
#include <wx/textctrl.h>

class AgentMqttConnection : public mosqpp::mosquittopp {
public:
    static AgentMqttConnection* GetInstance();
    void SendCommand(const std::string& command);

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
};

#endif // INNERSTAT_CLIENT_AGENT_MQTT_CONNECTION_H