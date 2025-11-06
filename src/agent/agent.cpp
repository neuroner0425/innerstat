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

// Function to get password without echoing to the console
std::string getPassword() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string password;
    char ch;
    while ((ch = getchar()) != '\n' && ch != '\r') {
        if (ch == 127) { // Handle backspace
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password.push_back(ch);
            std::cout << '*';
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return password;
}

// Function to check if sudo is possible with a password
bool checkSudo(const std::string& password) {
    std::string command = "echo '" + password + "' | sudo -S -v";
    int result = system(command.c_str());
    return result == 0;
}

std::string runCommand(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Could not execute command.";
    }

    char buffer[512];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);
    return result;
}

class Agent : public mosqpp::mosquittopp {
public:
    Agent(const char* id, const char* host, int port);
    ~Agent();

    void on_connect(int rc) override;
    void on_message(const struct mosquitto_message* message) override;
    void lsof_routine();

private:
    bool sudo_mode = false;
};

Agent::Agent(const char* id, const char* host, int port) : mosquittopp(id) {
    // Check for sudo with lsof without a password
    if (system("sudo -n lsof -i -P > /dev/null 2>&1") == 0) {
        sudo_mode = true;
        std::cout << "Sudo privileges detected. Running in Sudo Mode." << std::endl;
    } else {
        // Ask for password
        int attempts = 0;
        while (attempts < 3) {
            std::cout << "Enter password for sudo: ";
            std::string password = getPassword();

            if (checkSudo(password)) {
                sudo_mode = true;
                std::cout << "Password accepted. Running in Sudo Mode." << std::endl;
                break;
            } else {
                attempts++;
                std::cout << "Incorrect password. Please try again. (" << attempts << "/3)" << std::endl;
            }
        }
    }

    if (!sudo_mode) {
        std::cout << "Could not obtain sudo privileges. Running in Restricted Mode." << std::endl;
    }

    connect(host, port, 60);
}

Agent::~Agent() {
}

void Agent::on_connect(int rc) {
    if (rc == 0) {
        std::cout << "Agent connected to broker." << std::endl;
        subscribe(NULL, "innerstat/command");
    } else {
        std::cerr << "Agent connection failed." << std::endl;
    }
}

void Agent::on_message(const struct mosquitto_message* message) {
    std::string topic = message->topic;
    std::string payload = std::string((char*)message->payload, message->payloadlen);

    if (topic == "innerstat/command" && payload == "ps") {
        std::string result = runCommand("ps -a");
        publish(NULL, "innerstat/ps", result.length(), result.c_str());
    }
}

std::string parseLsof(const std::string& lsof_output) {
    std::map<std::string, std::set<std::string>> port_processes;
    std::istringstream iss(lsof_output);
    std::string line;
    bool first_line = true;

    while (std::getline(iss, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }

        std::istringstream line_stream(line);
        std::string cmd, pid, user, fd, type, device, size_off, node, name;

        line_stream >> cmd >> pid >> user >> fd >> type >> device >> size_off >> node >> name;

        size_t colon_pos = name.rfind(':');
        if (colon_pos != std::string::npos) {
            std::string port_info = name.substr(colon_pos + 1);
            size_t space_pos = port_info.find(' ');
            if (space_pos != std::string::npos) {
                port_info = port_info.substr(0, space_pos);
            }
            if (!port_info.empty() && port_info.back() == ')') {
                port_info.pop_back();
            }

            port_processes[port_info].insert(cmd);
        }
    }

    std::ostringstream result;
    for (const auto& pair : port_processes) {
        result << "PORT: " << pair.first << std::endl;
        for (const auto& process : pair.second) {
            result << "  " << process << std::endl;
        }
    }
    return result.str();
}

void Agent::lsof_routine(){
    while(true){
        std::string lsof_command = sudo_mode ? "sudo lsof -PiTCP -sTCP:LISTEN" : "lsof -PiTCP -sTCP:LISTEN";
        std::string result = runCommand(lsof_command);
        std::string parsed_result = parseLsof(result);
        publish(NULL, "innerstat/lsof", parsed_result.length(), parsed_result.c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    mosqpp::lib_init();
    Agent agent("innerstat_agent", "localhost", 1883);
    
    std::thread lsof_thread(&Agent::lsof_routine, &agent);

    agent.loop_forever();
    mosqpp::lib_cleanup();

    lsof_thread.join();

    return 0;
}