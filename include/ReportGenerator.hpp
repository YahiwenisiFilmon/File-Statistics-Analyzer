#pragma once

#include "FileSystemAnalyzer.hpp"
#include "LogAnalyzer.hpp"
#include <string>

namespace analyzer {

class ReportGenerator {
public:
    virtual ~ReportGenerator() = default;
    
    virtual std::string generate_fs_report(const DirectoryStats& stats) const = 0;
    virtual std::string generate_log_report(const LogSummary& summary) const = 0;
};

class TextReportGenerator : public ReportGenerator {
public:
    std::string generate_fs_report(const DirectoryStats& stats) const override;
    std::string generate_log_report(const LogSummary& summary) const override;
private:
    std::string format_size(uint64_t bytes) const;
};

class JsonReportGenerator : public ReportGenerator {
public:
    std::string generate_fs_report(const DirectoryStats& stats) const override;
    std::string generate_log_report(const LogSummary& summary) const override;
};

} // namespace analyzer
