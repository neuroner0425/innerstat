#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>

#ifndef INNERSTAT_COMMON_SYSTEM_INFO_H_
#define INNERSTAT_COMMON_SYSTEM_INFO_H_

/**
 * @brief Represents a single entry in the output of the `lsof` command.
 * 
 */
class LsofItem
{
public:
    std::string cmd;
    std::string pid;
    std::string user;
    std::string fd;
    std::string type;
    std::string device;
    std::string size;
    std::string node;
    std::string name;
    LsofItem(std::string& data){ deserialize(data); };
    LsofItem(const std::string& cmd,
             const std::string& pid,
             const std::string& user,
             const std::string& fd,
             const std::string& type,
             const std::string& device,
             const std::string& size,
             const std::string& node,
             const std::string& name);
    ~LsofItem(){};

    std::string serialize() const {
        std::ostringstream oss;
        oss << cmd << " " << pid << " " << user << " " << fd << " "
            << type << " " << device << " " << size << " " << node << " " << name;
        return oss.str();
    }

    void deserialize(const std::string& data) {
        std::istringstream line_stream(data);
        line_stream >> cmd >> pid >> user >> fd >> type >> device >> size >> node;
        std::getline(line_stream, name);
        // Remove possible leading space from name due to preceding extraction ops
        if (!name.empty() && name.front() == ' ') name.erase(0, 1);
    }
};

LsofItem::LsofItem(const std::string& cmd_,
                   const std::string& pid_,
                   const std::string& user_,
                   const std::string& fd_,
                   const std::string& type_,
                   const std::string& device_,
                   const std::string& size_,
                   const std::string& node_,
                    const std::string& name_)
    : cmd(cmd_), pid(pid_), user(user_), fd(fd_), type(type_), device(device_), size(size_), node(node_), name(name_)
{}


/**
 * @brief Represents the status of the system.
 * 
 */
class systemStatus
{
public:
    std::string mac_address;
    double cpu_usage;
    systemStatus(): mac_address(""), cpu_usage(0.0){};
    systemStatus(std::string mac, double cpu): mac_address(mac), cpu_usage(cpu){};
    ~systemStatus(){};

    std::string serialize() const {
        std::ostringstream oss;
        // use comma delimiter to avoid spaces in mac interfering
        oss << mac_address << "," << cpu_usage;
        return oss.str();
    }

    void deserialize(const std::string &data) {
        auto comma_pos = data.find(',');
        if (comma_pos == std::string::npos) {
            // Fallback: treat whole string as mac, cpu=0
            mac_address = data;
            cpu_usage = 0.0;
            return;
        }
        mac_address = data.substr(0, comma_pos);
        std::string cpu_str = data.substr(comma_pos + 1);
        cpu_usage = std::strtod(cpu_str.c_str(), nullptr);
    }
};

/**
 * @brief Represents the information of the system.
 * 
 */
class systemInfo
{
public:
    systemStatus status;
    std::vector<LsofItem> lsof_items;

    systemInfo(std::string &data){ deserialize(data); };
    systemInfo(std::string mac, double cpu, std::vector<LsofItem> &items)
        : status(mac, cpu), lsof_items(items) {}
    systemInfo(systemStatus stat, std::vector<LsofItem> &items)
        : status(stat), lsof_items(items) {}
    ~systemInfo(){};

    std::string serialize() const {
        std::ostringstream oss;
        // First line: systemStatus
        oss << status.serialize() << '\n';
        // Second line: number of lsof items
        oss << lsof_items.size() << '\n';
        // Subsequent lines: each LsofItem
        for (const auto &item : lsof_items) {
            oss << item.serialize() << '\n';
        }
        return oss.str();
    }
    void deserialize(const std::string& data) {
        lsof_items.clear();
        std::istringstream iss(data);
        std::string line;
        // First line: systemStatus
        if (!std::getline(iss, line)) return;
        status.deserialize(line);
        // Second line: count
        if (!std::getline(iss, line)) return;
        size_t count = std::strtoull(line.c_str(), nullptr, 10);
        // Next count lines: items
        for (size_t i = 0; i < count && std::getline(iss, line); ++i) {
            if (line.empty()) continue;
            LsofItem item(line);
            lsof_items.push_back(std::move(item));
        }
    }
};


#endif // INNERSTAT_COMMON_SYSTEM_INFO_H_
