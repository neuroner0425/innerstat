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
#include "innerstat/agent/command_execute.h"
#include "innerstat/agent/mac_address.h"
#include "innerstat/agent/check_sudo.h"
#include "innerstat/common/system_info.h"
#include "innerstat/agent/system_load_info.h"
#include "innerstat/agent/single_instance_guard.h"

std::vector<int> parseJsonPortList(const std::string& payload) {
    std::vector<int> ports;
    std::string numbers = payload;
    if (numbers.length() > 2) {
        numbers = numbers.substr(1, numbers.length() - 2);
    }

    std::stringstream ss(numbers);
    std::string item;
    while (std::getline(ss, item, ',')) {
        try {
            ports.push_back(std::stoi(item));
        } catch (...) {
            // ignore invalid numbers
        }
    }
    return ports;
}

class Agent : public mosqpp::mosquittopp {
private:
    bool sudo_mode = false;
    std::string mac_address;
    bool connected_ = false; // on_connect에서 설정되는 플래그
    bool loop_started_ = false; // loop_start 호출 여부
    
    std::map<int, long> tracked_ports_; // Port -> Last Request Timestamp (seconds)

    void publishSystemInfo();
    void updateTrackedPorts(const std::string& payload);
    void checkPortTTL();
    void checkForLogUpdates();

public:
    Agent(const char* id);
    ~Agent(){};

    void on_connect(int rc) override; // 연결 성공 시 호출
    void on_disconnect(int rc) override; // 연결 해제 시 호출
    void on_message(const struct mosquitto_message* message) override; // 메시지 수신 시 호출
    void agent_routine(); // 에이전트 주기적 작업
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
        std::string command_topic = "innerstat/" + this->mac_address + "/command";
        std::string request_topic = "innerstat/" + this->mac_address + "/req";
        subscribe(NULL, command_topic.c_str());
        subscribe(NULL, request_topic.c_str());
        subscribe(NULL, "innerstat/broadcast/request_info");
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
    std::string command_topic = "innerstat/" + this->mac_address + "/command";
    std::string request_topic = "innerstat/" + this->mac_address + "/req";

    if (topic == command_topic && payload == "ps") {
        std::string result = runCommand("ps -a");
        std::string ps_topic = "innerstat/" + this->mac_address + "/ps";
        publish(NULL, ps_topic.c_str(), result.length(), result.c_str());
    } else if (topic == "innerstat/broadcast/request_info") {
        publishSystemInfo();
    } else if (topic == request_topic) {
        updateTrackedPorts(payload);
    }
}

void Agent::updateTrackedPorts(const std::string& payload) {
    std::vector<int> ports = parseJsonPortList(payload);
    long current_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    for (int port : ports) {
        tracked_ports_[port] = current_time;
    }
}

void Agent::checkPortTTL() {
    long current_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    const int ttl = 20; // 20 seconds
    for (auto it = tracked_ports_.begin(); it != tracked_ports_.end(); ) {
        if (current_time - it->second > ttl) {
            it = tracked_ports_.erase(it);
        } else {
            ++it;
        }
    }
}

void Agent::checkForLogUpdates() {
    // This is a placeholder for the complex file monitoring logic.
    // For now, it will generate a fake log for a random tracked port every so often.
    if (tracked_ports_.empty()) {
        return;
    }

    int random_chance = rand() % 100;
    if (random_chance < 20) { // 20% chance each second
        int port_index = rand() % tracked_ports_.size();
        auto it = tracked_ports_.begin();
        std::advance(it, port_index);
        int port = it->first;

        std::string log_topic = "innerstat/" + mac_address + "/" + std::to_string(port) + "/log";
        std::string log_message = "This is a simulated log message for port " + std::to_string(port);
        publish(NULL, log_topic.c_str(), log_message.length(), log_message.c_str());
    }
}

void Agent::publishSystemInfo() {
    double cpu_load = get_cpu_usage();
    std::vector<LsofItem> parsed_result = getPSbyPort(sudo_mode);
    std::string os = "Unknown";
#ifdef _WIN32
    os = "Windows";
#elif __APPLE__
    os = "macOS";
#elif __linux__
    os = "Linux";
#endif
    systemStatus status(mac_address, cpu_load, os);
    systemInfo info(status, parsed_result);
    std::string serialized_result = info.serialize();
    std::string info_topic = "innerstat/" + this->mac_address + "/info";
    publish(NULL, info_topic.c_str(), serialized_result.length(), serialized_result.c_str());
}

void Agent::agent_routine(){
    srand(time(0)); // Seed for random log generation
    while(true){
        publishSystemInfo();
        checkPortTTL();
        checkForLogUpdates();
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