#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <map>
#include <sstream>
#include <set>
#include <mosquittopp.h>
#include <chrono>
#include <thread>
#include <format>
#include "innerstat/agent/command_execute.h"
#include "innerstat/agent/mac_address.h"
#include "innerstat/agent/check_sudo.h"
#include "innerstat/common/system_info.h"
#include "innerstat/agent/system_load_info.h"
#include "innerstat/agent/single_instance_guard.h"

class Agent : public mosqpp::mosquittopp {
private:
    bool sudo_mode = false;
    std::string mac_address;
    bool connected_ = false; // on_connect에서 설정되는 플래그
    bool loop_started_ = false; // loop_start 호출 여부

public:
    Agent(const char* id);
    ~Agent(){};

    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_message(const struct mosquitto_message* message) override;
    void agent_routine();
    bool interactive_connect();
};

Agent::Agent(const char* id) : mosquittopp(id) {
    sudo_mode = getSudoPrivileges();
    if (sudo_mode) std::cout << "Running in Sudo Mode." << std::endl;
    else std::cout << "Running in Restricted Mode." << std::endl;
    
    mac_address = get_mac_address();
    std::cout << "MAC Address: " << mac_address << std::endl;
}

void Agent::on_connect(int rc) {
    if (rc == 0) {
        std::cout << "Agent connected to broker." << std::endl;
        subscribe(NULL, "innerstat/command");
        connected_ = true;
    } else {
        std::cerr << "Agent connection failed." << std::endl;
        connected_ = false;
    }
}

void Agent::on_disconnect(int rc) {
    std::cerr << "Agent disconnected (rc=" << rc << ")." << std::endl;
    connected_ = false;
}

void Agent::on_message(const struct mosquitto_message* message) {
    std::string topic = message->topic;
    std::string payload = std::string((char*)message->payload, message->payloadlen);

    if (topic == "innerstat/command" && payload == "ps") {
        std::string result = runCommand("ps -a");
        publish(NULL, std::string("innerstat/" + mac_address + "/systemInfo").c_str(), result.length(), result.c_str());
    }
}

void Agent::agent_routine(){
    while(true){
        double cpu_load = get_cpu_usage();
        std::vector<LsofItem> parsed_result = getPSbyPort(sudo_mode);
        systemStatus status(mac_address, cpu_load);
        systemInfo info(status, parsed_result);
        std::string serialized_result = info.serialize();
        publish(NULL, "innerstat/lsof", serialized_result.length(), serialized_result.c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool Agent::interactive_connect(){
    while(true){
        std::string host;
        std::string port_input;
        int port = 1883;

        std::cout << "Enter Mosquitto Broker Host (default: localhost): ";
        if(!std::getline(std::cin, host)) return false; // EOF
        std::cout << "Enter Mosquitto Broker Port (default: 1883): ";
        if(!std::getline(std::cin, port_input)) return false; // EOF

        if (host.empty()) host = "localhost";
        if (!port_input.empty()) {
            try {
                port = std::stoi(port_input);
                if(port <= 0 || port > 65535){
                    std::cerr << "Invalid port range." << std::endl;
                    continue;
                }
            } catch(const std::exception&){
                std::cerr << "Invalid port number." << std::endl;
                continue;
            }
        }

        if(!loop_started_){
            int lrc = loop_start();
            if(lrc != MOSQ_ERR_SUCCESS){
                std::cerr << "Failed to start loop thread (rc=" << lrc << ")." << std::endl;
                return false;
            }
            loop_started_ = true;
        }

        connected_ = false;
        int rc = connect_async(host.c_str(), port);
        if(rc != MOSQ_ERR_SUCCESS){
            std::cerr << "Connect initiation failed (rc=" << rc << ")." << std::endl;
            continue;
        }

        for(int i=0; i<50 && !connected_; ++i){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if(connected_){
            std::cout << "Connection established to " << host << ":" << port << std::endl;
            return true;
        }
        std::cerr << "Unable to connect (timeout or refused). Try again." << std::endl;
        disconnect();
    }
    return false;
}

int main() {
    std::cout << "Starting innerstat agent..." << std::endl;

    // Check single instance
    SingleInstanceGuard instance_guard("/tmp/innerstat_agent.lock");
    if (!instance_guard.acquired()) {
        std::cerr << "another agent is already running." << std::endl;
        return 1;
    }

    // Initialize Mosquitto library
    mosqpp::lib_init();
    Agent agent("innerstat_agent");
    if(!agent.interactive_connect()){
        std::cerr << "Interactive connect aborted." << std::endl;
        mosqpp::lib_cleanup();
        return 1;
    }
    
    std::thread lsof_thread(&Agent::agent_routine, &agent);

    lsof_thread.join();

    mosqpp::lib_cleanup();
    return 0;
}