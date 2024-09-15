#ifndef ROCKETLINK_API_CALLBACKS_HPP
#define ROCKETLINK_API_CALLBACKS_HPP

#include <functional>
#include <mutex>
#include "AVC/Command.hpp"
#include "AVC/Telemetry.hpp"
#include "Diagnostics/Diagnostics.hpp"

namespace RocketLink {
namespace API {

/**
 * @brief The Callbacks class provides hooks for users to respond to various RocketLink events.
 * 
 * Users can register callback functions to handle incoming commands, telemetry data,
 * and diagnostic updates. These callbacks are invoked by the RocketLink system when 
 * the corresponding events occur.
 */
class Callbacks {
public:
    using CommandCallback = std::function<void(const RocketLink::AVC::Command&)>;
    using TelemetryCallback = std::function<void(const RocketLink::AVC::Telemetry&)>;
    using DiagnosticsCallback = std::function<void(const RocketLink::Diagnostics::Diagnostics&)>;

    /**
     * @brief Sets the callback function to be invoked when a Command is received.
     * @param cb The callback function.
     */
    void setCommandCallback(CommandCallback cb);

    /**
     * @brief Sets the callback function to be invoked when Telemetry data is received.
     * @param cb The callback function.
     */
    void setTelemetryCallback(TelemetryCallback cb);

    /**
     * @brief Sets the callback function to be invoked when Diagnostics data is updated.
     * @param cb The callback function.
     */
    void setDiagnosticsCallback(DiagnosticsCallback cb);

    /**
     * @brief Invokes the registered Command callback with the provided Command data.
     *        If no callback is registered, the function does nothing.
     * @param command The Command data.
     */
    void invokeCommandCallback(const RocketLink::AVC::Command& command);

    /**
     * @brief Invokes the registered Telemetry callback with the provided Telemetry data.
     *        If no callback is registered, the function does nothing.
     * @param telemetry The Telemetry data.
     */
    void invokeTelemetryCallback(const RocketLink::AVC::Telemetry& telemetry);

    /**
     * @brief Invokes the registered Diagnostics callback with the provided Diagnostics data.
     *        If no callback is registered, the function does nothing.
     * @param diagnostics The Diagnostics data.
     */
    void invokeDiagnosticsCallback(const RocketLink::Diagnostics::Diagnostics& diagnostics);

private:
    CommandCallback commandCallback_;
    TelemetryCallback telemetryCallback_;
    DiagnosticsCallback diagnosticsCallback_;

    std::mutex mutex_;
};

} // namespace API
} // namespace RocketLink

#endif // ROCKETLINK_API_CALLBACKS_HPP
