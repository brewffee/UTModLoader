// todo: create test mod files for each format

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "config.h"
#include "util.h"
#include "commands/extract.h"

using str = std::string;

int main(const int argc, char* argv[]) {
    const std::string usage_str = "Usage: utmodloader [options] <command> <path>\n"
        "Options:\n"
        "\t-h, --help       Display this help message\n"
        "\t-v, --version    Display version information\n"
        "Commands:\n"
        "\textract          Extract mod files. Accepts a directory or a single file path";

    if (argc < 2) {
        std::cout << "No command specified.\n\n" << usage_str << std::endl;
        return EXIT_FAILURE;
    }

    const std::vector<std::string> args(argv, argv + argc);

    // Check if flags are present anywhere in args
    if (find_in(args, "-h") || find_in(args, "--help")) {
        std::cout << usage_str << std::endl;
        return EXIT_SUCCESS;
    }

    if (find_in(args, "-v") || find_in(args, "--version")) {
        std::cout << "UTModLoader v" << PROJECT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    // Subcommands
    for (int i = 1; i < args.size(); i++) {
        if (args[i] == "extract") {
            if (i + 1 < args.size()) {
                // 'extract' takes no flags, and accepts one argument, so
                // there's no need to check before running
                return extract_mods(args[i + 1]);
            }

            std::cerr << "No path specified" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
