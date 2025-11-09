#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include <cctype>

#include "innerstat/common/json_util.h"

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

    std::string serialize() const;
    void deserialize(const std::string& data);
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

    std::string serialize() const;
    void deserialize(const std::string &data);
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

    std::string serialize() const;
    void deserialize(const std::string& data);
};

// ---- Implementations ----
inline std::string LsofItem::serialize() const {
    std::ostringstream oss;
    using innerstat_json_util::escape;
    oss << '{'
        << "\"cmd\":\"" << escape(cmd) << "\",";
    oss << "\"pid\":\"" << escape(pid) << "\",";
    oss << "\"user\":\"" << escape(user) << "\",";
    oss << "\"fd\":\"" << escape(fd) << "\",";
    oss << "\"type\":\"" << escape(type) << "\",";
    oss << "\"device\":\"" << escape(device) << "\",";
    oss << "\"size\":\"" << escape(size) << "\",";
    oss << "\"node\":\"" << escape(node) << "\",";
    oss << "\"name\":\"" << escape(name) << "\"";
    oss << '}';
    return oss.str();
}

inline void LsofItem::deserialize(const std::string &data) {
    using namespace innerstat_json_util;
    extractString(data, "cmd", cmd);
    extractString(data, "pid", pid);
    extractString(data, "user", user);
    extractString(data, "fd", fd);
    extractString(data, "type", type);
    extractString(data, "device", device);
    extractString(data, "size", size);
    extractString(data, "node", node);
    extractString(data, "name", name);
}

inline std::string systemStatus::serialize() const {
    std::ostringstream oss;
    using innerstat_json_util::escape;
    oss << '{'
        << "\"mac_address\":\"" << escape(mac_address) << "\",";
    oss << "\"cpu_usage\":" << cpu_usage;
    oss << '}';
    return oss.str();
}

inline void systemStatus::deserialize(const std::string &data) {
    using namespace innerstat_json_util;
    extractString(data, "mac_address", mac_address);
    std::string cpu_str;
    if (extractString(data, "cpu_usage", cpu_str)) {
        cpu_usage = std::strtod(cpu_str.c_str(), nullptr);
    }
}

inline std::string systemInfo::serialize() const {
    std::ostringstream oss;
    oss << '{';
    oss << "\"status\":" << status.serialize() << ',';
    oss << "\"lsof_items\":" << '[';
    for (size_t i = 0; i < lsof_items.size(); ++i) {
        if (i) oss << ',';
        oss << lsof_items[i].serialize();
    }
    oss << ']';
    oss << '}';
    return oss.str();
}

inline void systemInfo::deserialize(const std::string &data) {
    lsof_items.clear();
    // Find status object
    size_t status_key = data.find("\"status\"");
    if (status_key != std::string::npos) {
        size_t colon = data.find(':', status_key);
        if (colon != std::string::npos) {
            size_t brace_start = data.find('{', colon);
            if (brace_start != std::string::npos) {
                int depth = 0; size_t i = brace_start;
                for (; i < data.size(); ++i) {
                    if (data[i] == '{') ++depth; else if (data[i] == '}') { --depth; if (depth == 0) { ++i; break; } }
                }
                std::string status_obj = data.substr(brace_start, i - brace_start);
                status.deserialize(status_obj);
            }
        }
    }
    // Find lsof_items array
    size_t items_key = data.find("\"lsof_items\"");
    if (items_key != std::string::npos) {
        size_t colon = data.find(':', items_key);
        if (colon != std::string::npos) {
            size_t arr_start = data.find('[', colon);
            if (arr_start != std::string::npos) {
                int depth = 0; size_t i = arr_start;
                for (; i < data.size(); ++i) {
                    if (data[i] == '[') ++depth; else if (data[i] == ']') { --depth; if (depth == 0) { ++i; break; } }
                }
                std::string arr_content = data.substr(arr_start + 1, (i - arr_start - 2 < data.size() ? i - arr_start - 2 : 0));
                // Iterate objects inside array
                size_t pos = 0;
                while (pos < arr_content.size()) {
                    // Skip whitespace and commas
                    while (pos < arr_content.size() && (std::isspace(static_cast<unsigned char>(arr_content[pos])) || arr_content[pos] == ',')) ++pos;
                    if (pos >= arr_content.size()) break;
                    if (arr_content[pos] != '{') break; // malformed
                    int obj_depth = 0; size_t start = pos;
                    for (; pos < arr_content.size(); ++pos) {
                        if (arr_content[pos] == '{') ++obj_depth; else if (arr_content[pos] == '}') { --obj_depth; if (obj_depth == 0) { ++pos; break; } }
                    }
                    std::string obj = arr_content.substr(start, pos - start);
                    LsofItem item(obj);
                    lsof_items.push_back(std::move(item));
                }
            }
        }
    }
}


#endif // INNERSTAT_COMMON_SYSTEM_INFO_H_
