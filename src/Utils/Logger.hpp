#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <string>
#include <map>
#include <chrono>
#include <iomanip>
#include <thread>

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    // Retrieves the singleton instance of Logger
    static Logger& getInstance();

    // Logs a message with the specified log level
    void log(LogLevel level, const std::string& message);

    // Sets the minimum log level for messages to be logged
    void setLogLevel(LogLevel level);

    // Adds an output stream to the logger
    void addOutput(std::ostream& output);

    // Enables or disables timestamp in log messages
    void enableTimestamp(bool enable);

    // Enables or disables thread ID in log messages
    void enableThreadId(bool enable);

    // Deletes copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();  // Private constructor for singleton
    ~Logger(); // Destructor

    // Formats the log message
    std::string formatMessage(LogLevel level, const std::string& message);

    // Converts LogLevel to string representation
    std::string logLevelToString(LogLevel level);

    LogLevel minLogLevel;                          // Minimum log level to log
    bool includeTimestamp;                         // Flag to include timestamp
    bool includeThreadId;                          // Flag to include thread ID
    std::vector<std::ostream*> outputStreams;       // Vector of output streams
    std::mutex logMutex;                           // Mutex for thread safety
    std::map<LogLevel, std::string> logLevelMap;    // Mapping from LogLevel to string
};

#endif // LOGGER_HPP
