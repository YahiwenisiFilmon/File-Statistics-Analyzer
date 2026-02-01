#include "ReportGenerator.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace analyzer {

using json = nlohmann::json;

std::string TextReportGenerator::format_size(uint64_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = static_cast<double>(bytes);
    while (size >= 1024 && i < 4) {
        size /= 1024;
        i++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[i];
    return oss.str();
}

std::string TextReportGenerator::generate_fs_report(const DirectoryStats& stats) const {
    std::ostringstream oss;
    oss << "========================================\n";
    oss << "      FILE SYSTEM ANALYSIS REPORT       \n";
    oss << "========================================\n\n";
    
    oss << "Summary:\n";
    oss << "  Total Files:       " << stats.total_files << "\n";
    oss << "  Total Directories: " << stats.total_directories << "\n";
    oss << "  Total Size:        " << format_size(stats.total_size) << "\n\n";

    oss << "File Type Distribution (Top 10):\n";
    std::vector<std::pair<std::string, uint64_t>> types(stats.type_distribution_count.begin(), stats.type_distribution_count.end());
    std::sort(types.begin(), types.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (size_t i = 0; i < std::min(types.size(), size_t(10)); ++i) {
        oss << "  " << std::left << std::setw(15) << types[i].first << ": " << types[i].second << " files (" 
            << format_size(stats.type_distribution_size.at(types[i].first)) << ")\n";
    }
    oss << "\n";

    oss << "Size Distribution:\n";
    for (const auto& range : stats.size_histogram) {
        oss << "  " << std::left << std::setw(15) << range.label << ": " << range.count << " files\n";
    }
    oss << "\n";

    oss << "Age Distribution:\n";
    for (const auto& range : stats.age_distribution) {
        oss << "  " << std::left << std::setw(15) << range.label << ": " << range.count << " files\n";
    }
    oss << "\n";

    oss << "Largest Files:\n";
    auto largest = stats.largest_files;
    std::sort(largest.begin(), largest.end(), [](const auto& a, const auto& b) { return a.size > b.size; });
    for (const auto& entry : largest) {
        oss << "  " << format_size(entry.size) << "  " << entry.path.string() << "\n";
    }
    oss << "\n";

    oss << "Oldest Files:\n";
    auto oldest = stats.oldest_files;
    std::sort(oldest.begin(), oldest.end(), [](const auto& a, const auto& b) { return a.last_modified < b.last_modified; });
    for (const auto& entry : oldest) {
        oss << "  " << entry.path.string() << "\n";
    }

    return oss.str();
}

std::string TextReportGenerator::generate_log_report(const LogSummary& summary) const {
    std::ostringstream oss;
    oss << "========================================\n";
    oss << "          LOG ANALYSIS REPORT           \n";
    oss << "========================================\n\n";

    oss << "Summary:\n";
    oss << "  Total Requests:    " << summary.total_requests << "\n";
    oss << "  Total Data Sent:   " << format_size(summary.total_bytes) << "\n";
    oss << "  Unique IPs:        " << summary.unique_ips << "\n";
    oss << "  Error Rate:        " << std::fixed << std::setprecision(2) << (summary.error_rate * 100) << "%\n\n";

    oss << "HTTP Status Distribution:\n";
    for (const auto& [status, count] : summary.status_code_stats) {
        oss << "  " << status << ": " << count << " requests\n";
    }
    oss << "\n";

    oss << "HTTP Method Breakdown:\n";
    for (const auto& [method, count] : summary.method_stats) {
        oss << "  " << std::left << std::setw(10) << method << ": " << count << "\n";
    }
    oss << "\n";

    oss << "Top Endpoints:\n";
    std::vector<std::pair<std::string, uint64_t>> endpoints(summary.endpoint_stats.begin(), summary.endpoint_stats.end());
    std::sort(endpoints.begin(), endpoints.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    for (size_t i = 0; i < std::min(endpoints.size(), size_t(10)); ++i) {
        oss << "  " << std::left << std::setw(30) << endpoints[i].first << ": " << endpoints[i].second << "\n";
    }
    oss << "\n";

    oss << "Top IP Addresses:\n";
    std::vector<std::pair<std::string, uint64_t>> ips(summary.ip_stats.begin(), summary.ip_stats.end());
    std::sort(ips.begin(), ips.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    for (size_t i = 0; i < std::min(ips.size(), size_t(10)); ++i) {
        oss << "  " << std::left << std::setw(30) << ips[i].first << ": " << ips[i].second << "\n";
    }

    if (summary.regex_match_count > 0) {
        oss << "\nRegex Pattern Matches: " << summary.regex_match_count << "\n";
    }

    return oss.str();
}

std::string JsonReportGenerator::generate_fs_report(const DirectoryStats& stats) const {
    json j;
    j["summary"]["total_files"] = stats.total_files;
    j["summary"]["total_directories"] = stats.total_directories;
    j["summary"]["total_size"] = stats.total_size;
    
    j["type_distribution"] = stats.type_distribution_count;
    
    for (const auto& range : stats.size_histogram) {
        j["size_distribution"][range.label] = range.count;
    }
    
    for (const auto& entry : stats.largest_files) {
        j["largest_files"].push_back({
            {"path", entry.path.string()},
            {"size", entry.size},
            {"extension", entry.extension}
        });
    }
    
    return j.dump(4);
}

std::string JsonReportGenerator::generate_log_report(const LogSummary& summary) const {
    json j;
    j["summary"]["total_requests"] = summary.total_requests;
    j["summary"]["total_bytes"] = summary.total_bytes;
    j["summary"]["unique_ips"] = summary.unique_ips;
    j["summary"]["error_rate"] = summary.error_rate;

    j["status_codes"] = summary.status_code_stats;
    j["methods"] = summary.method_stats;
    j["top_endpoints"] = summary.endpoint_stats;
    
    return j.dump(4);
}

} // namespace analyzer
