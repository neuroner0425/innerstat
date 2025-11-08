#include "innerstat/agent/elevation.h"

#if defined(_WIN32)
  #include <windows.h>
  #include <shellapi.h>
  #include <vector>
  #include <sstream>
#endif


#if defined(_WIN32)

static bool isProcessElevatedWindows() {
    BOOL isMember = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        return false;
    }
    if (!CheckTokenMembership(nullptr, adminGroup, &isMember)) {
        isMember = FALSE;
    }
    FreeSid(adminGroup);
    return isMember == TRUE;
}

static std::wstring quoteArg(const std::wstring& s) {
    bool needQuotes = s.find_first_of(L" \t\"\n\r") != std::wstring::npos;
    if (!needQuotes) return s;
    std::wstring out = L"\"";
    for (wchar_t c : s) {
        if (c == L'"') out += L"\\\""; else out += c;
    }
    out += L"\"";
    return out;
}

static std::wstring getExePath() {
    std::wstring buf(260, L'\0');
    DWORD len = 0;
    for (;;) {
        len = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        if (len == 0) return L"";
        if (len < buf.size()) { buf.resize(len); return buf; }
        buf.resize(buf.size() * 2);
    }
}

static std::wstring rebuildArgsFromCurrentProcess() {
    int argcW = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
    if (!argvW || argcW <= 1) {
        if (argvW) LocalFree(argvW);
        return L"";
    }
    std::wostringstream oss;
    for (int i = 1; i < argcW; ++i) {
        if (i > 1) oss << L' ';
        oss << quoteArg(argvW[i]);
    }
    LocalFree(argvW);
    return oss.str();
}

bool isElevated() {
    return isProcessElevatedWindows();
}

bool ensureElevated(int /*argc*/, char* /*argv*/[]) {
    if (isProcessElevatedWindows()) return true;

    std::wstring exe = getExePath();
    std::wstring params = rebuildArgsFromCurrentProcess();

    HINSTANCE res = ShellExecuteW(nullptr, L"runas", exe.c_str(),
                                  params.empty() ? nullptr : params.c_str(),
                                  nullptr, SW_SHOWNORMAL);
    if ((INT_PTR)res <= 32) {
        // User cancelled or failure
        return false;
    }
    // Elevated instance launched; caller should exit current process
    return false;
}

#else // POSIX (macOS/Linux)

bool isElevated() {
    // No concept of runas here in this utility; consider sudo elsewhere.
    return true;
}

bool ensureElevated(int /*argc*/, char* /*argv*/[]) {
    return true; // No-op on POSIX
}

#endif