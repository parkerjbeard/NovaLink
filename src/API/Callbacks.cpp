#include "API/Callbacks.hpp"

namespace RocketLink {
namespace API {

void Callbacks::setCommandCallback(CommandCallback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    commandCallback_ = std::move(cb);
}

void Callbacks::setTelemetryCallback(TelemetryCallback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    telemetryCallback_ = std::move(cb);
}

void Callbacks::setDiagnosticsCallback(DiagnosticsCallback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    diagnosticsCallback_ = std::move(cb);
}

void Callbacks::invokeCommandCallback(const RocketLink::AVC::Command& command) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (commandCallback_) {
        commandCallback_(command);
    }
}

void Callbacks::invokeTelemetryCallback(const RocketLink::AVC::Telemetry& telemetry) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (telemetryCallback_) {
        telemetryCallback_(telemetry);
    }
}

void Callbacks::invokeDiagnosticsCallback(const RocketLink::Diagnostics::Diagnostics& diagnostics) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (diagnosticsCallback_) {
        diagnosticsCallback_(diagnostics);
    }
}

} // namespace API
} // namespace RocketLink
