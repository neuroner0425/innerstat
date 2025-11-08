#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <iomanip>

#if defined(_WIN32)
    #include <windows.h>
    #include <Pdh.h>
#elif defined(__APPLE__)
    #include <mach/mach.h>
    #include <mach/mach_host.h>
#elif defined(__linux__)
    #include <fstream>
    #include <sstream>
#endif

double get_cpu_usage() {

#if defined(_WIN32) // Windows
    static PDH_HQUERY query = nullptr;
    static PDH_HCOUNTER counter = nullptr;

    if (query == nullptr) {
        if (PdhOpenQuery(NULL, NULL, &query) != ERROR_SUCCESS) return 0.0;
        if (PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &counter) != ERROR_SUCCESS) {
            PdhCloseQuery(query); query = nullptr; return 0.0;
        }
        PdhCollectQueryData(query);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (PdhCollectQueryData(query) != ERROR_SUCCESS) return 0.0;

    PDH_FMT_COUNTERVALUE value;
    if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &value) != ERROR_SUCCESS) return 0.0;

    return std::max(0.0, std::min(100.0, value.doubleValue));

#elif defined(__APPLE__) // macOS
    host_cpu_load_info_data_t t1, t2;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    auto get_mac_cpu_info = [&](host_cpu_load_info_data_t& info) -> bool {
        return host_statistics64(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info64_t)&info, &count) == KERN_SUCCESS;
    };

    if (!get_mac_cpu_info(t1)) return 0.0;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!get_mac_cpu_info(t2)) return 0.0;

    unsigned long long user_diff = t2.cpu_ticks[CPU_STATE_USER] - t1.cpu_ticks[CPU_STATE_USER];
    unsigned long long system_diff = t2.cpu_ticks[CPU_STATE_SYSTEM] - t1.cpu_ticks[CPU_STATE_SYSTEM];
    unsigned long long nice_diff = t2.cpu_ticks[CPU_STATE_NICE] - t1.cpu_ticks[CPU_STATE_NICE];
    unsigned long long idle_diff = t2.cpu_ticks[CPU_STATE_IDLE] - t1.cpu_ticks[CPU_STATE_IDLE];

    unsigned long long total_diff = user_diff + system_diff + nice_diff + idle_diff;

    if (total_diff == 0) return 0.0;

    double usage = 100.0 * (total_diff - idle_diff) / (double)total_diff;
    return std::max(0.0, std::min(100.0, usage));

#elif defined(__linux__) // Linux
    struct CPU_Times {
        long long user = 0, nice = 0, system = 0, idle = 0;
        long long total() const { return user + nice + system + idle; }
    };

    auto read_cpu_times = []() -> CPU_Times {
        CPU_Times times = {};
        std::ifstream file("/proc/stat");
        std::string line;
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cpu;
            ss >> cpu >> times.user >> times.nice >> times.system >> times.idle;
        }
        return times;
    };

    CPU_Times t1 = read_cpu_times();
    if (t1.total() == 0) return 0.0;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CPU_Times t2 = read_cpu_times();

    long long total_diff = t2.total() - t1.total();
    long long idle_diff = t2.idle - t1.idle;

    if (total_diff == 0) return 0.0;

    double usage = 100.0 * (total_diff - idle_diff) / total_diff;
    return std::max(0.0, std::min(100.0, usage));

#else // Unsupported OS
    std::cerr << "Error: Unsupported OS." << std::endl;
    return 0.0;
#endif
}