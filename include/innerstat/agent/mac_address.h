#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm> // std::transform

#ifdef _WIN32
    #include <winsock2.h>
    #include <iphlpapi.h>
    #pragma comment(lib, "Iphlpapi.lib")
    #pragma comment(lib, "Ws2_32.lib")
#elif defined(__linux__) || defined(__APPLE__)
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <cstring>
    #include <arpa/inet.h> // inet_ntoa
    
    #ifdef __APPLE__
        #include <ifaddrs.h>
        #include <net/if_dl.h>
    #else
        #include <ifaddrs.h>
    #endif
#endif

#ifndef INNERSTAT_AGENT_MAC_ADDRESS_H_
#define INNERSTAT_AGENT_MAC_ADDRESS_H_

std::string get_mac_address() {
    std::string mac_address = "MAC_ADDRESS_NOT_FOUND";

    #ifdef _WIN32
    ULONG buffer_size = sizeof(IP_ADAPTER_ADDRESSES);
    std::vector<BYTE> buffer(buffer_size);
    PIP_ADAPTER_ADDRESSES adapter_addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    // 필요한 크기를 얻거나 데이터를 가져옴
    DWORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &buffer_size);
    if (result == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(buffer_size);
        adapter_addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
        result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &buffer_size);
    }

    if (result == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES adapter = adapter_addresses; adapter != nullptr; adapter = adapter->Next) {
            // 1. MAC 주소가 유효한지 확인 (6바이트)
            if (adapter->PhysicalAddressLength == 6) {
                // 2. IPv4 주소가 할당되었는지 확인
                bool has_ipv4 = false;
                for (PIP_ADAPTER_UNICAST_ADDRESS addr = adapter->FirstUnicastAddress; addr != nullptr; addr = addr->Next) {
                    if (addr->Address.lpSockaddr->sa_family == AF_INET) {
                        // 루프백(127.0.0.1) 주소는 제외하는 것이 좋지만, 여기서는 단순히 존재 여부만 확인
                        has_ipv4 = true;
                        break; 
                    }
                }

                if (has_ipv4) {
                    std::stringstream ss;
                    for (DWORD i = 0; i < adapter->PhysicalAddressLength; ++i) {
                        if (i != 0) ss << ":";
                        ss << std::hex << std::setw(2) << std::setfill('0') 
                           << static_cast<int>(adapter->PhysicalAddress[i]);
                    }
                    mac_address = ss.str();
                    std::transform(mac_address.begin(), mac_address.end(), mac_address.begin(), ::toupper);
                    return mac_address; // IPv4와 MAC이 모두 유효한 첫 번째 어댑터 반환
                }
            }
        }
    }

    #elif defined(__linux__) || defined(__APPLE__)
    struct ifaddrs *ifap, *ifa;
    std::string current_mac;
    bool ipv4_found = false;

    if (getifaddrs(&ifap) == 0) {
        for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) continue;

            // 1. MAC 주소(AF_LINK) 추출
            if (ifa->ifa_addr->sa_family == AF_LINK) {
                struct sockaddr_dl *sdl = reinterpret_cast<struct sockaddr_dl*>(ifa->ifa_addr);
                if (sdl->sdl_alen == 6) {
                    std::stringstream ss;
                    unsigned char *mac_data = (unsigned char *)LLADDR(sdl);
                    for (int i = 0; i < 6; ++i) {
                        if (i != 0) ss << ":";
                        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac_data[i]);
                    }
                    current_mac = ss.str();
                    ipv4_found = false; // 새 인터페이스 시작 시 초기화
                }
            }
            
            // 2. IPv4 주소(AF_INET) 확인
            else if (ifa->ifa_addr->sa_family == AF_INET && !current_mac.empty()) {
                // 루프백 주소 (127.0.0.1) 제외
                struct sockaddr_in *sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
                if (ntohl(sa->sin_addr.s_addr) != INADDR_LOOPBACK) {
                    ipv4_found = true;
                }
            }

            // 3. IPv4와 MAC 주소가 모두 발견되면 반환
            if (ipv4_found && !current_mac.empty()) {
                std::transform(current_mac.begin(), current_mac.end(), current_mac.begin(), ::toupper);
                mac_address = current_mac;
                freeifaddrs(ifap);
                return mac_address; 
            }
        }
        freeifaddrs(ifap);
    }

    #endif

    return mac_address; // MAC 주소를 찾지 못했을 경우
}

#endif  // INNERSTAT_AGENT_MAC_ADDRESS_H_