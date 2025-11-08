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

#include "innerstat/common/system_info.h"

#ifndef INNERSTAT_AGENT_COMMAND_EXECUTE_H_
#define INNERSTAT_AGENT_COMMAND_EXECUTE_H_

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

// Function to parse lsof output and return a vector of LsofItems
inline std::vector<LsofItem> parseLsof(const std::string& lsof_output) {
    std::vector<LsofItem> items;
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

            items.emplace_back(cmd, pid, user, fd, type, device, size_off, node, name);
        }
    }

    return items;
}

std::vector<LsofItem> getPSbyPort(bool sudo_mode){
    std::string lsof_command = sudo_mode ? "sudo lsof -PiTCP -sTCP:LISTEN" : "lsof -PiTCP -sTCP:LISTEN";
    std::string result = runCommand(lsof_command);
    return parseLsof(result);
}

#endif // INNERSTAT_AGENT_COMMAND_EXECUTE_H_