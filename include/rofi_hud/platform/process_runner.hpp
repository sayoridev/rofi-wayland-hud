#pragma once

#include <string>

namespace rofi_hud {

// Result of a captured shell command execution.
struct CommandResult {
    std::string output;  // combined stdout of the command
    int exit_code{-1};   // -1 when the exit status could not be determined
    bool spawned{false}; // false when popen() itself failed (e.g. fork error)
};

// Runs `command` through /bin/sh and captures its stdout.
// The caller is responsible for redirecting stderr (e.g. "2>/dev/null"), matching
// the behavior of the previous inline popen() call sites.
CommandResult run_command(const std::string& command);

} // namespace rofi_hud
