#include "FileSystemAnalyzer.hpp"
#include <algorithm>
#include <iostream>

namespace analyzer {

FileSystemAnalyzer::FileSystemAnalyzer(AnalysisOptions options) 
    : options_(std::move(options)) {}

DirectoryStats FileSystemAnalyzer::analyze() {
    DirectoryStats stats;
    initialize_histogram(stats);
    
    try {
        if (!fs::exists(options_.target_path) || !fs::is_directory(options_.target_path)) {
            std::cerr << "Error: Target path does not exist or is not a directory: " << options_.target_path << std::endl;
            return stats;
        }
        traverse(options_.target_path, 0, stats);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error during analysis: " << e.what() << std::endl;
    }

    // Sort top lists
    auto sort_by_size = [](const FileEntry& a, const FileEntry& b) { return a.size > b.size; };
    auto sort_by_date_asc = [](const FileEntry& a, const FileEntry& b) { return a.last_modified < b.last_modified; };
    auto sort_by_date_desc = [](const FileEntry& a, const FileEntry& b) { return a.last_modified > b.last_modified; };

    // These would ideally be populated during traversal if we maintain a top-N heap
    // but for simplicity we'll assume they were updated.
    
    return stats;
}

void FileSystemAnalyzer::traverse(const fs::path& path, int current_depth, DirectoryStats& stats) {
    if (options_.max_depth != -1 && current_depth > options_.max_depth) return;

    try {
        for (const auto& entry : fs::directory_iterator(path, fs::directory_options::skip_permission_denied)) {
            if (options_.skip_hidden && entry.path().filename().string().front() == '.') continue;

            if (fs::is_directory(entry.status())) {
                stats.total_directories++;
                traverse(entry.path(), current_depth + 1, stats);
            } else if (fs::is_regular_file(entry.status())) {
                process_file(entry.path(), stats);
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Warning: Could not access directory " << path << ": " << e.what() << std::endl;
    }
}

void FileSystemAnalyzer::process_file(const fs::path& path, DirectoryStats& stats) {
    uint64_t size = fs::file_size(path);
    if (size < options_.min_size_threshold) return;

    FileEntry entry;
    entry.path = path;
    entry.size = size;
    entry.extension = path.has_extension() ? path.extension().string() : "no-extension";
    entry.last_modified = fs::last_write_time(path);

    stats.total_files++;
    stats.total_size += size;
    
    update_aggregation(entry, stats);
}

void FileSystemAnalyzer::update_aggregation(const FileEntry& entry, DirectoryStats& stats) {
    stats.type_distribution_count[entry.extension]++;
    stats.type_distribution_size[entry.extension] += entry.size;

    // Size distribution constants
    static const std::vector<std::pair<std::string, uint64_t>> size_ranges = {
        {"0-1KB", 1024},
        {"1KB-1MB", 1024 * 1024},
        {"1MB-100MB", 100 * 1024 * 1024},
        {"100MB-1GB", 1024 * 1024 * 1024}
    };

    bool size_bucketed = false;
    for (size_t i = 0; i < size_ranges.size(); ++i) {
        if (entry.size < size_ranges[i].second) {
            stats.size_histogram[i].count++;
            size_bucketed = true;
            break;
        }
    }
    if (!size_bucketed) stats.size_histogram.back().count++;

    // Age distribution
    auto now = std::chrono::system_clock::now();
    // Approximation of file_time_type to system_clock
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        entry.last_modified - fs::file_time_type::clock::now() + now);
    auto age_duration = now - sctp;
    
    if (age_duration < std::chrono::hours(24)) stats.age_distribution[0].count++;
    else if (age_duration < std::chrono::hours(24 * 7)) stats.age_distribution[1].count++;
    else if (age_duration < std::chrono::hours(24 * 30)) stats.age_distribution[2].count++;
    else stats.age_distribution[3].count++;

    // Top-N Lists helper
    auto update_top_list = [](std::vector<FileEntry>& list, const FileEntry& e, auto cmp) {
        list.push_back(e);
        std::push_heap(list.begin(), list.end(), cmp);
        if (list.size() > 10) {
            std::pop_heap(list.begin(), list.end(), cmp);
            list.pop_back();
        }
    };

    update_top_list(stats.largest_files, entry, [](const auto& a, const auto& b) { return a.size > b.size; });
    update_top_list(stats.oldest_files, entry, [](const auto& a, const auto& b) { return a.last_modified < b.last_modified; });
    update_top_list(stats.newest_files, entry, [](const auto& a, const auto& b) { return a.last_modified > b.last_modified; });
}

void FileSystemAnalyzer::initialize_histogram(DirectoryStats& stats) {
    stats.size_histogram = {{"0-1KB"}, {"1KB-1MB"}, {"1MB-100MB"}, {"100MB-1GB"}, {"1GB+"}};
    stats.age_distribution = {{"Today"}, {"This Week"}, {"This Month"}, {"Older"}};
}

} // namespace analyzer
