#include "rofi_hud/platform/process_runner.hpp"
#include <array>
#include <cstdio>
#include <sys/wait.h>

namespace rofi_hud {

CommandResult run_command(const std::string& command) {
    CommandResult result;

    FILE* pipe = ::popen(command.c_str(), "r");
    if (pipe == nullptr) {
        return result; // spawned == false
    }

    std::array<char, 512> buffer{};
    while (::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result.output += buffer.data();
    }

    const int status = ::pclose(pipe);
    result.spawned = true;
    if (status != -1 && WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    }
    return result;
}

} // namespace rofi_hud
