#pragma once

// Cross-platform elevation helper
// - Windows: if not elevated, relaunch self via ShellExecute("runas")
// - macOS/Linux: no-op (returns true)

#include <string>

namespace innerstat {
namespace agent {

// Returns true if the current process already has elevated/admin privileges.
bool isElevated();

// Ensure elevated privileges.
// Windows: If not elevated, launches a new elevated instance of the current executable
//          with the same command-line arguments, then returns false in the original
//          process (so caller can exit). If already elevated, returns true.
// POSIX: Always returns true (no relaunch performed).
bool ensureElevated(int argc, char* argv[]);

} // namespace agent
} // namespace innerstat
