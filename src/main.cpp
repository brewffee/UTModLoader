// todo: create test mod files for each format

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "config.h"
#include "commands/extract.h"
#include "util/container.h"

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

    // This program only accepts global flags, so we can check here safely
    if (find_in(args, "-h") || find_in(args, "--help")) {
        std::cout << usage_str << std::endl;
        return EXIT_SUCCESS;
    }

    if (find_in(args, "-v") || find_in(args, "--version")) {
        std::cout << "UTModLoader v" << PROJECT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    // Subcommands
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "extract") {
            if (i + 1 < args.size()) {
                return extract_mods(args[i + 1]);
            }

            std::cerr << "No path specified" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
