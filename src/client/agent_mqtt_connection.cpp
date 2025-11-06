#include "innerstat/client/agent_mqtt_connection.h"
#include <wx/wx.h>
#include <wx/wx.h>

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
        subscribe(NULL, "innerstat/lsof");
        subscribe(NULL, "innerstat/ps");
    } else {
        wxLogError("Client connection failed.");
    }
}

void AgentMqttConnection::on_message(const struct mosquitto_message* message) {
    std::string topic = message->topic;
    std::string payload = std::string((char*)message->payload, message->payloadlen);

    if (topic == "innerstat/lsof") {
        if (lsofText && eventHandler) {
            eventHandler->CallAfter([this, payload]() {
                lsofText->SetValue(payload);
            });
        }
    } else if (topic == "innerstat/ps") {
        if (psText && eventHandler) {
            eventHandler->CallAfter([this, payload]() {
                psText->SetValue(payload);
            });
        }
    }
}

void AgentMqttConnection::SendCommand(const std::string& command) {
    publish(NULL, "innerstat/command", command.length(), command.c_str());
}