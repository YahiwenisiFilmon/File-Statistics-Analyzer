#include "LogAnalyzer.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <set>
#include <nlohmann/json.hpp>

namespace analyzer {

using json = nlohmann::json;

LogAnalyzer::LogAnalyzer(LogFormat format) : format_(format) {}

LogSummary LogAnalyzer::analyze(const std::string& file_path, const LogFilterOptions& options) {
    LogSummary summary;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open log file: " << file_path << std::endl;
        return summary;
    }

    std::string line;
    uint64_t error_count = 0;
    std::set<std::string> unique_ips;
    std::optional<std::regex> search_regex;
    if (!options.pattern_regex.empty()) {
        search_regex = std::regex(options.pattern_regex, std::regex::icase);
    }

    while (std::getline(file, line)) {
        if (search_regex && std::regex_search(line, *search_regex)) {
            summary.regex_match_count++;
            if (summary.matched_lines.size() < 100) summary.matched_lines.push_back(line);
        }

        LogEntry entry;
        if (!parse_line(line, entry)) continue;

        // Apply filters
        if (options.error_only && entry.status_code < 400) continue;
        if (!options.status_codes.empty()) {
            if (std::find(options.status_codes.begin(), options.status_codes.end(), entry.status_code) == options.status_codes.end()) continue;
        }
        
        // Time filtering (simplified - would need actual timestamp parsing)
        // ...

        summary.total_requests++;
        summary.total_bytes += entry.body_bytes_sent;
        unique_ips.insert(entry.ip);
        
        summary.ip_stats[entry.ip]++;
        summary.endpoint_stats[entry.endpoint]++;
        summary.status_code_stats[entry.status_code]++;
        summary.method_stats[entry.method]++;

        if (entry.status_code >= 400) {
            error_count++;
            summary.top_errors[entry.endpoint]++;
        }
    }

    summary.unique_ips = unique_ips.size();
    if (summary.total_requests > 0) {
        summary.error_rate = static_cast<double>(error_count) / summary.total_requests;
    }

    return summary;
}

bool LogAnalyzer::parse_line(const std::string& line, LogEntry& entry) {
    if (line.empty()) return false;
    
    // Auto-detect if needed
    if (format_ == LogFormat::AutoDetect) {
        if (line.front() == '{') return parse_json(line, entry);
        return parse_apache_common(line, entry);
    } else if (format_ == LogFormat::Json) {
        return parse_json(line, entry);
    } else {
        return parse_apache_common(line, entry);
    }
}

bool LogAnalyzer::parse_apache_common(const std::string& line, LogEntry& entry) {
    // Regex for Apache Common Log Format: 127.0.0.1 - frank [10/Oct/2000:13:55:36 -0700] "GET /apache_pb.gif HTTP/1.0" 200 2326
    static const std::regex apache_regex(R"(^(\S+) \S+ \S+ \[([^\]]+)\] "(\S+) (\S+) \S+" (\d+) (\d+|-))");
    std::smatch match;
    
    if (std::regex_search(line, match, apache_regex)) {
        entry.ip = match[1];
        entry.timestamp_str = match[2];
        entry.method = match[3];
        entry.endpoint = match[4];
        entry.status_code = std::stoi(match[5]);
        entry.body_bytes_sent = (match[6] == "-") ? 0 : std::stoull(match[6]);
        return true;
    }
    return false;
}

bool LogAnalyzer::parse_json(const std::string& line, LogEntry& entry) {
    try {
        auto j = json::parse(line);
        entry.ip = j.value("ip", "");
        entry.method = j.value("method", "");
        entry.endpoint = j.value("endpoint", j.value("url", ""));
        entry.status_code = j.value("status", j.value("status_code", 0));
        entry.body_bytes_sent = j.value("size", j.value("bytes", 0));
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace analyzer
