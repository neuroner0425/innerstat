#include <iostream>
#include <string>

#if defined(_WIN32)
    #include <windows.h>
    #include <securitybaseapi.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <cstdio>
#endif

#ifndef INNERSTAT_AGENT_CHECK_SUDO_H_
#define INNERSTAT_AGENT_CHECK_SUDO_H_

#if defined(_WIN32)

// Windows: no sudo. Check if process is elevated (Administrator).
inline bool getSudoPrivileges() {
    BOOL isMember = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        std::cout << "Administrator privileges not available. ";
        return false;
    }
    if (!CheckTokenMembership(nullptr, adminGroup, &isMember)) {
        isMember = FALSE;
    }
    FreeSid(adminGroup);

    if (isMember) {
        std::cout << "Administrator privileges detected. ";
        return true;
    }
    std::cout << "Administrator privileges not available. ";
    return false;
}

#else

// POSIX: get password without echoing to the console
inline std::string getPassword() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string password;
    int ch;
    while ((ch = getchar()) != '\n' && ch != '\r' && ch != EOF) {
        if (ch == 127 || ch == 8) { // Handle backspace (DEL or BS)
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password.push_back(static_cast<char>(ch));
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return password;
}

// Escape single quotes for POSIX shell within single-quoted string
inline std::string escapeForSingleQuotes(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\'') {
            out += "'\''"; // close ', insert escaped ', reopen '
        } else {
            out += c;
        }
    }
    return out;
}

// Function to check if sudo is possible with a password (no prompt)
inline bool checkSudoWithPassword(const std::string& password) {
    const std::string pwEsc = escapeForSingleQuotes(password);
    std::string command = "echo '" + pwEsc + "' | sudo -S -p '' -v 2>/dev/null";
    return system(command.c_str()) == 0;
}

// Main function to check for sudo privileges on POSIX
inline bool getSudoPrivileges(){
    // Try non-interactive first (cached credentials / NOPASSWD)
    if (system("sudo -n -v >/dev/null 2>&1") == 0) {
        std::cout << "Sudo privileges detected. ";
        return true;
    }

    int attempts = 0;
    while (attempts < 3) {
        std::cout << "Enter password for sudo: " << std::flush;
        std::string password = getPassword();
        if (checkSudoWithPassword(password)) {
            std::cout << "Password accepted. ";
            return true;
        }
        attempts++;
        std::cout << "Incorrect password. Please try again. (" << attempts << "/3)" << std::endl;
    }

    std::cout << "Could not obtain sudo privileges. ";
    return false;
}

#endif
#endif