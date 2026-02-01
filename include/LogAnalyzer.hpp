#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <chrono>

namespace analyzer {

enum class LogFormat {
    ApacheCommon,
    ApacheCombined,
    NginxCombined,
    Json,
    AutoDetect
};

struct LogEntry {
    std::string ip;
    std::string timestamp_str;
    std::string method;
    std::string endpoint;
    int status_code = 0;
    uint64_t body_bytes_sent = 0;
    std::string referer;
    std::string user_agent;
    
    // Parsed timestamp
    std::chrono::system_clock::time_point timestamp;
};

struct LogSummary {
    uint64_t total_requests = 0;
    uint64_t total_bytes = 0;
    uint32_t unique_ips = 0;
    
    std::map<std::string, uint64_t> ip_stats;           // IP -> count
    std::map<std::string, uint64_t> endpoint_stats;     // endpoint -> count
    std::map<int, uint64_t> status_code_stats;          // status -> count
    std::map<std::string, uint64_t> method_stats;       // method -> count
    
    double error_rate = 0.0;
    std::map<std::string, uint64_t> top_errors;         // endpoint -> error count

    // New spec requirements
    std::vector<std::string> matched_lines;
    uint64_t regex_match_count = 0;
};

struct LogFilterOptions {
    std::optional<std::chrono::system_clock::time_point> start_time;
    std::optional<std::chrono::system_clock::time_point> end_time;
    std::vector<int> status_codes;
    std::string pattern_regex;
    bool error_only = false;
};

class LogAnalyzer {
public:
    explicit LogAnalyzer(LogFormat format = LogFormat::AutoDetect);
    
    LogSummary analyze(const std::string& file_path, const LogFilterOptions& options = {});

private:
    bool parse_line(const std::string& line, LogEntry& entry);
    bool parse_apache_common(const std::string& line, LogEntry& entry);
    bool parse_json(const std::string& line, LogEntry& entry);
    
    LogFormat format_;
};

} // namespace analyzer
