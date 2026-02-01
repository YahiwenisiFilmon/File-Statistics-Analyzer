#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace analyzer {

struct FileEntry {
    fs::path path;
    uint64_t size;
    std::string extension;
    fs::file_time_type last_modified;
};

struct DirectoryStats {
    uint64_t total_files = 0;
    uint64_t total_directories = 0;
    uint64_t total_size = 0;
    
    std::map<std::string, uint64_t> type_distribution_count; // ext -> count
    std::map<std::string, uint64_t> type_distribution_size;  // ext -> total size
    
    // Size distribution (histogram)
    struct Range {
        std::string label;
        uint64_t count = 0;
    };
    std::vector<Range> size_histogram;
    std::vector<Range> age_distribution;

    std::vector<FileEntry> largest_files;
    std::vector<FileEntry> oldest_files;
    std::vector<FileEntry> newest_files;
};

struct AnalysisOptions {
    fs::path target_path;
    int max_depth = -1; // -1 for infinite
    std::vector<std::string> include_patterns;
    std::vector<std::string> exclude_patterns;
    uint64_t min_size_threshold = 0;
    bool skip_hidden = true;
};

class FileSystemAnalyzer {
public:
    explicit FileSystemAnalyzer(AnalysisOptions options);
    
    DirectoryStats analyze();

private:
    void traverse(const fs::path& path, int current_depth, DirectoryStats& stats);
    void process_file(const fs::path& path, DirectoryStats& stats);
    void update_aggregation(const FileEntry& entry, DirectoryStats& stats);
    
    AnalysisOptions options_;
    void initialize_histogram(DirectoryStats& stats);
};

} // namespace analyzer
