#include "Logger.hpp"

// Initializes the singleton instance
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Private constructor
Logger::Logger()
    : minLogLevel(LogLevel::DEBUG),
      includeTimestamp(true),
      includeThreadId(true) {
    // Initialize log level map
    logLevelMap[LogLevel::DEBUG] = "DEBUG";
    logLevelMap[LogLevel::INFO] = "INFO";
    logLevelMap[LogLevel::WARNING] = "WARNING";
    logLevelMap[LogLevel::ERROR] = "ERROR";
    logLevelMap[LogLevel::CRITICAL] = "CRITICAL";

    // By default, add std::cout as an output stream
    outputStreams.push_back(&std::cout);
}

// Destructor
Logger::~Logger() {
    // No dynamic resources to clean up
}

// Sets the minimum log level
void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    minLogLevel = level;
}

// Adds an output stream
void Logger::addOutput(std::ostream& output) {
    std::lock_guard<std::mutex> lock(logMutex);
    outputStreams.push_back(&output);
}

// Enables or disables timestamp
void Logger::enableTimestamp(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex);
    includeTimestamp = enable;
}

// Enables or disables thread ID
void Logger::enableThreadId(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex);
    includeThreadId = enable;
}

// Logs a message with the given log level
void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLogLevel) {
        return;
    }

    std::string formattedMessage = formatMessage(level, message);

    std::lock_guard<std::mutex> lock(logMutex);
    for (auto& stream : outputStreams) {
        if (stream && *stream) {
            (*stream) << formattedMessage << std::endl;
        }
    }
}

// Formats the log message
std::string Logger::formatMessage(LogLevel level, const std::string& message) {
    std::ostringstream oss;

    if (includeTimestamp) {
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                now.time_since_epoch()) % 1000;

        oss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << milliseconds.count()
            << " ";
    }

    oss << "[" << logLevelToString(level) << "] ";

    if (includeThreadId) {
        oss << "[Thread " << std::this_thread::get_id() << "] ";
    }

    oss << message;

    return oss.str();
}

// Converts LogLevel to string
std::string Logger::logLevelToString(LogLevel level) {
    auto it = logLevelMap.find(level);
    if (it != logLevelMap.end()) {
        return it->second;
    }
    return "UNKNOWN";
}
