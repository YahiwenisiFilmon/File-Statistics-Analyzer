#include "FileSystemAnalyzer.hpp"
#include "LogAnalyzer.hpp"
#include "ReportGenerator.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

void print_usage() {
    std::cout << "Usage: FileStatAnalyzer <command> <path> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  fs <dir>     Analyze file system statistics\n";
    std::cout << "  log <file>   Analyze log file statistics\n\n";
    std::cout << "Options:\n";
    std::cout << "  --json       Output in JSON format (default: text)\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    std::string path = argv[2];
    bool use_json = false;
    int depth = -1;
    uint64_t min_size = 0;
    std::string regex_pattern;

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--json") {
            use_json = true;
        } else if (arg.starts_with("--depth=") && arg.length() > 8) {
            depth = std::stoi(arg.substr(8));
        } else if (arg.starts_with("--min-size=") && arg.length() > 11) {
            min_size = std::stoull(arg.substr(11));
        } else if (arg.starts_with("--regex=") && arg.length() > 8) {
            regex_pattern = arg.substr(8);
        }
    }

    std::unique_ptr<analyzer::ReportGenerator> generator;
    if (use_json) {
        generator = std::make_unique<analyzer::JsonReportGenerator>();
    } else {
        generator = std::make_unique<analyzer::TextReportGenerator>();
    }

    if (command == "fs") {
        analyzer::AnalysisOptions options;
        options.target_path = path;
        options.max_depth = depth;
        options.min_size_threshold = min_size;
        analyzer::FileSystemAnalyzer analyzer(options);
        auto stats = analyzer.analyze();
        std::cout << generator->generate_fs_report(stats) << std::endl;
    } else if (command == "log") {
        analyzer::LogAnalyzer analyzer;
        analyzer::LogFilterOptions options;
        options.pattern_regex = regex_pattern;
        auto summary = analyzer.analyze(path, options);
        std::cout << generator->generate_log_report(summary) << std::endl;
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        print_usage();
        return 1;
    }

    return 0;
}
